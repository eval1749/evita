// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

testing.test('ImageData.decode.bmp', function(t) {
  const data = base.atob(
      'Qk06AAAAAAAAADYAAAAoAAAAAQAAAAEAAAABABgAAAAAAAQAAA' +
      'AAAAAAAAAAAAAAAAAAAAAAAQEBAA==');
  const image = ImageData.decode('image/bmp', data);
  t.expect(Array.from(image.data)).toEqual([1, 1, 1, 255]);
  t.expect(image.width).toEqual(1);
  t.expect(image.height).toEqual(1);
});

testing.test('ImageData.decode.jpeg', function(t) {
  const data = base.atob(
      '/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDAAMCAgICAgMCAgIDAwMDBAYEB' +
      'AQEBAgGBgUGCQgKCgkICQkKDA8MCgsOCwkJDRENDg8QEBEQCgwSExIQEw8' +
      'QEBD/2wBDAQMDAwQDBAgEBAgQCwkLEBAQEBAQEBAQEBAQEBAQEBAQEBAQE' +
      'BAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBD/wAARCAABAAEDASIAAhE' +
      'BAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAA' +
      'gEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRV' +
      'S0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZW' +
      'mNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWmp6ipqrK' +
      'ztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+' +
      'Pn6/8QAHwEAAwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREAAgE' +
      'CBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzU' +
      'vAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldYWVp' +
      'jZGVmZ2hpanN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqs' +
      'rO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn' +
      '6/9oADAMBAAIRAxEAPwD48oooroOc/9k=');
  const image = ImageData.decode('image/jpeg', data);
  t.expect(Array.from(image.data)).toEqual([51, 56, 60, 255]);
  t.expect(image.width).toEqual(1);
  t.expect(image.height).toEqual(1);
});

testing.test('ImageData.decode.png', function(t) {
  const data = base.atob(
      'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUl' +
      'EQVQIW2NMyatoAgAEVgHOjOuCYQAAAABJRU5ErkJggg==');
  const image = ImageData.decode('image/png', data);
  t.expect(Array.from(image.data)).toEqual([100, 110, 120, 130]);
  t.expect(image.width).toEqual(1);
  t.expect(image.height).toEqual(1);
});
