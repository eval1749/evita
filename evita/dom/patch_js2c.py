#!/usr/bin/env python

import sys;

FOOTER = """\
namespace dom {

void LoadJsLibrary(v8::Isolate* isolate) {
  typedef v8::internal::NativesCollection<v8::internal::D8> Lib;
  v8::HandleScope handle_scope(isolate);
  auto const num_files = Lib::GetBuiltinsCount();
  for (auto source_index = 0; source_index < num_files; ++source_index) {
    auto shell_source = Lib::GetRawScriptSource(source_index);
    auto shell_source_name = Lib::GetScriptName(source_index);
    auto source = v8::String::NewFromUtf8(isolate, shell_source.start(),
                                     v8::String::kNormalString,
                                     shell_source.length());
    auto name = v8::String::NewFromUtf8(isolate, shell_source_name.start(),
                                    v8::String::kNormalString,
                                    shell_source_name.length());
    v8::Script::Compile(source, name)->Run();
  }
}

}  // namespace dom
"""

WARNINGS = [
    4100, 4127, 4189, 4244, 4245, 4302, 4512, 4800
];

def main():
  input_file = sys.argv[1];
  output_file = sys.argv[2];

  lines = open(input_file, "rt").readlines();

  output = open(output_file, "w");
  output.write("#pragma warning(disable: %(warnings)s)\n" % {
    'warnings': " ".join(map(str, WARNINGS)),
  })
  for line in lines:
    line = line.replace('#include "', '#include "v8/src/');
    output.write(line);
  output.write(FOOTER);
  output.close()

if __name__ == "__main__":
  main()
