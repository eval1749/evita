// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

#include "evita/visuals/demo/demo_application.h"

#include "base/at_exit.h"
#include "base/bind.h"
#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/process/launch.h"
#include "base/run_loop.h"
#include "build/build_config.h"
#include "base/strings/stringprintf.h"
#include "evita/ui/animation/animation_scheduler.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/visuals/demo/demo_window.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/model/box_builder.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/paint/painter.h"
#include "evita/visuals/paint/paint_info.h"
#include "evita/visuals/style/style.h"
#include "evita/visuals/style/style_builder.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// BoxPrinter
//
class BoxPrinter final : public BoxVisitor {
 public:
  BoxPrinter() = default;
  ~BoxPrinter() final = default;

 private:
  void Indent() const;

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  int indent_ = 0;

  DISALLOW_COPY_AND_ASSIGN(BoxPrinter);
};

void BoxPrinter::Indent() const {
  for (auto i = indent_; i > 0; --i)
    std::cout << "  ";
}

void BoxPrinter::VisitBlockBox(BlockBox* box) {
  Indent();
  const auto& style = box->ComputeActualStyle();
  std::cout << *box << ' ' << *style << std::endl;
  ++indent_;
  for (const auto& child : box->child_boxes())
    Visit(child);
  --indent_;
}

void BoxPrinter::VisitLineBox(LineBox* box) {
  Indent();
  const auto& style = box->ComputeActualStyle();
  std::cout << *box << ' ' << *style << std::endl;
  ++indent_;
  for (const auto& child : box->child_boxes())
    Visit(child);
  --indent_;
}

void BoxPrinter::VisitTextBox(TextBox* box) {
  Indent();
  std::cout << *box << " \"";
  for (const auto& char_code : box->text()) {
    if (char_code < 0x20 || char_code >= 0x7F) {
      std::cout << base::StringPrintf("\\u%04X", char_code);
      continue;
    }
    if (char_code == '"' || char_code == '\\')
      std::cout << '\\';
    std::cout << static_cast<char>(char_code);
  }
  const auto& style = box->ComputeActualStyle();
  std::cout << '"' << ' ' << *style << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// DemoMain
//
void RunDemo() {
  BoxBuilder builder(BoxBuilder::New<BlockBox>());
  builder.SetStyle(
      *StyleBuilder().SetBackground(Background(FloatColor(1, 1, 1))).Build());
  const auto& kBlack = StyleBuilder().SetColor(FloatColor(0, 0, 0)).Build();
  for (auto index = 0; index < 15; ++index) {
    auto line_builder = BoxBuilder::New<LineBox>();
    line_builder.Append(BoxBuilder::New<TextBox>(
                            base::StringPrintf(L"line %d", index))
                            .SetStyle(*kBlack)
                            .Finish())
        .Append(BoxBuilder::New<TextBox>(L"size").SetStyle(*kBlack).Finish())
        .Append(BoxBuilder::New<TextBox>(L"status").SetStyle(*kBlack).Finish())
        .Append(BoxBuilder::New<TextBox>(L"file").SetStyle(*kBlack).Finish());
    builder.Append(line_builder.Finish());
  }

  const auto& viewport_bounds = FloatRect(FloatSize(640, 480));
  const auto& root_box = builder.Finish();
  Layouter().Layout(root_box.get(), viewport_bounds);

  std::cout << "Box Tree:" << std::endl;
  BoxPrinter printer;
  printer.Visit(*root_box);

  std::cout << std::endl << "Display Item List:" << std::endl;
  PaintInfo paint_info(viewport_bounds);
  const auto& display_item_list = Painter().Paint(paint_info, *root_box);
  {
    auto indent = 0;
    for (const auto& item : display_item_list->items()) {
      if (item->is<EndClipDisplayItem>() ||
          item->is<EndTransformDisplayItem>()) {
        --indent;
      }
      for (auto count = indent; count > 0; --count)
        std::cout << "  ";
      std::cout << item << std::endl;
      if (item->is<BeginClipDisplayItem>() ||
          item->is<BeginTransformDisplayItem>()) {
        ++indent;
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////
//
// DemoScheduler
//
class DemoScheduler : public ui::AnimationScheduler {
 public:
  DemoScheduler() { ui::AnimationScheduler::GetInstance()->SetScheduler(this); }

  ~DemoScheduler() = default;

 private:
  void BeginFrame();

  // ui::AnimationScheduler
  void CancelAnimationFrameRequest(ui::AnimationFrameHandler* handler) final;
  void RequestAnimationFrame(ui::AnimationFrameHandler* handler) final;

  bool is_waiting_ = false;
  base::Time last_frame_time_;
  std::unordered_set<ui::AnimationFrameHandler*> pending_handlers_;

  DISALLOW_COPY_AND_ASSIGN(DemoScheduler);
};

void DemoScheduler::BeginFrame() {
  std::unordered_set<ui::AnimationFrameHandler*> ready_handlers;
  ready_handlers.swap(pending_handlers_);
  const auto& now = base::Time::Now();
  last_frame_time_ = now;
  for (const auto& handler : ready_handlers)
    handler->HandleAnimationFrame(now);
  ui::Compositor::instance()->CommitIfNeeded();
}

// ui::AnimationScheduler
void DemoScheduler::CancelAnimationFrameRequest(
    ui::AnimationFrameHandler* handler) {
  pending_handlers_.erase(handler);
}

void DemoScheduler::RequestAnimationFrame(ui::AnimationFrameHandler* handler) {
  pending_handlers_.insert(handler);
  if (is_waiting_)
    return;
  const auto next_frame_time =
      last_frame_time_ + base::TimeDelta::FromMilliseconds(1000 / 60);
  const auto delta = next_frame_time - base::Time::Now();
  base::MessageLoop::current()->task_runner()->PostNonNestableDelayedTask(
      FROM_HERE, base::Bind(&DemoScheduler::BeginFrame, base::Unretained(this)),
      std::min(base::TimeDelta::FromMilliseconds(3),
               std::max(delta, base::TimeDelta::FromMilliseconds(3))));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DemoApplication
//
DemoApplication::DemoApplication() {}
DemoApplication::~DemoApplication() {}

void DemoApplication::Run() {
  base::AtExitManager at_exit;
  DemoScheduler scheduler;
  base::RouteStdioToConsole(true);
  auto message_loop =
      std::make_unique<base::MessageLoop>(base::MessageLoop::TYPE_UI);
  base::RunLoop run_loop;
  const auto& window = new DemoWindow(run_loop.QuitClosure());
  window->RealizeWidget();
  RunDemo();
  run_loop.Run();
}

}  // namespace visuals
