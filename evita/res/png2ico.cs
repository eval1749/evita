using System;
using System.IO;

// http://msdn.microsoft.com/en-us/magazine/cc546571.aspx
// http://blogs.msdn.com/b/oldnewthing/archive/2010/10/18/10077133.aspx

class Png2Ico {

/*
struct ICONDIR {
  uint16_t idReserved;
  uint16_t idType; // 1=ICO, 2=CUR
  uint16_t idCount;
  ICONDIRENTRY idEntries[];
};

struct ICONDIRENTRY {
  uint8_t bWidth;
  uint8_t bHeight;
  uint8_t bColorCount;
  uint8_t bReserved;
  uint16_t wPlanes;
  uint16_t wBitCount;
  uint32_t dwBytesInRes;
  uint32_t dwImageOffset; // start of ICO file
};

*/

public static void Main(String[] argv) {
  foreach (var pngFileName in argv) {
    using (var pngStream = File.OpenRead(pngFileName)) {
      var iconBytes = new byte[pngStream.Length + 22];
      pngStream.Read(iconBytes, 22, (int) pngStream.Length);

      iconBytes[0] = 0; // Reserved 2byte
      iconBytes[1] = 0;
      iconBytes[2] = 1; // Reserved 2byte ICO=1, CUR=2
      iconBytes[3] = 0;
      iconBytes[4] = 1; // Image Count
      iconBytes[5] = 0;
      // Image Info[0]
      iconBytes[6] = 32; // width
      iconBytes[7] = 32; // height
      iconBytes[8] = 0; // color count
      iconBytes[9] = 0; // reserved always 0
      iconBytes[10] = 1; // reserved always 1
      iconBytes[11] = 0;
      iconBytes[12] = 32; // bits per pixel
      iconBytes[13] = 0;
      // image size in bytes (4 byte)
      iconBytes[14] = (byte)(pngStream.Length & 0xFF);
      iconBytes[15] = (byte) ((pngStream.Length >> 8) & 0xFF);
      iconBytes[16] = (byte) ((pngStream.Length >> 16) & 0xFF);
      iconBytes[17] = (byte) ((pngStream.Length >> 24) & 0xFF);
       // image offset (4 byte)
      iconBytes[18] = 22;
      iconBytes[19] = 0;
      iconBytes[20] = 0;
      iconBytes[21] = 0;

      var iconFileName = Path.GetFileNameWithoutExtension(pngFileName) + ".ico";
      using (var iconStream = File.Create(iconFileName)) {
        iconStream.Write(iconBytes, 0, iconBytes.Length);
      }
    }
  }
}

}
