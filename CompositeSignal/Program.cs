using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;

namespace CompositeSignal {
	class Program {

		static void Main(string[] args) {
			if (args.Length < 1 || string.IsNullOrWhiteSpace(args[0]) || !File.Exists(args[0])) {
				return;
			}
			var path = args[0];
			var img = new Bitmap(path);
			var width = img.Width;
			var height = img.Height;
			var data = img.LockBits(
				new Rectangle(0, 0, img.Width, img.Height),
				ImageLockMode.ReadOnly,
				PixelFormat.Format24bppRgb
			);
			var stride = data.Stride;
			var arrData = new byte[data.Stride * data.Height];
			Marshal.Copy(data.Scan0, arrData, 0, arrData.Length);
			img.UnlockBits(data);
			img.Dispose();

			if (true) {
				var outPath = Path.GetDirectoryName(path) + "\\" + Path.GetFileNameWithoutExtension(path);
				var sw = new FileStream(outPath, FileMode.Create);
				var yiq = new YIQ();
				var ptr = 0;
				for (int y = 0; y < height; y++) {
					int x = 0, p = ptr;
					while (x < width) {
						double output = 0;
						if (yiq.Encode(ref output, arrData, p, x == 0)) {
							x++;
							p += 3;
						}
						sw.WriteByte((byte)(output * 127 + 128));
					}
					ptr += stride;
				}
				sw.Close();
				sw.Dispose();
			}

			if (true) {
				var outPath = Path.GetDirectoryName(path) + "\\" + Path.GetFileNameWithoutExtension(path) + ".csv";
				var sw = new StreamWriter(outPath);
				var yiq = new YIQ();
				var ptr = 0;
				var s1 = 0.0;
				var s2 = 0.0;
				sw.WriteLine("Input,S0,Y,C");
				for (int y = 0; y < height && y < 2; y++) {
					int x = 0, p = ptr;
					while (x < width) {
						double output = 0;
						if (yiq.Encode(ref output, arrData, p, x == 0)) {
							x++;
							p += 3;
						}
						var d = (byte)(output * 127 + 128);
						var s0 = (d - 128) / 128.0;
						var lum = s0 + s2;
						var chroma = (s0 - s2) * 3 / 2;
						if (lum < 0) {
							lum *= -1;
						}
						sw.WriteLine("{0},{1},{2},{3}",
							d,
							s0.ToString("0.00"),
							lum.ToString("0.00"),
							chroma.ToString("0.00")
						);
						s2 = s1;
						s1 = s0;
					}
					ptr += stride;
				}
				sw.Close();
				sw.Dispose();
			}
		}

	}
}
