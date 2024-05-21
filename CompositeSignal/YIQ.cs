namespace CompositeSignal {
	class YIQ {
		const int SyncTriggerBegin = 8;
		const int SyncTriggerMid = 15;
		const int SyncTriggerEnd = 22;
		const int SyncChromaBegin = 30;
		const int SyncChromaEnd = 50;
		const int SyncSamples = 60;

		static readonly int[] Y = { 1, -1 };
		static readonly int[] Re = { 1, 0,-1, 0 };
		static readonly int[] Im = { 0, 1, 0, -1 };

		int mCount = 0;
		int mEnableSync = 0;
		int mIdxY = 0;
		int mIdxC = 0;
		int mOddEven = 0;

		double mT = 0;
		double mY = 0;
		double mCi = 0;
		double mCq = 0;
		double mChromaSyncAmp = 0;

		double mS1, mS2;

		public bool Encode(ref double output, byte[] pix, int ptr, bool sync) {
			if (0 == mEnableSync && sync) {
				mEnableSync = 1;
				mCount = 0;
			}

			int enableSignal;
			int enableSync;
			var trigger = 0.0;
			if (0 == mEnableSync) {
				/* 状態：信号出力 */
				enableSignal = 1;
				enableSync = 0;
			} else if (mCount < SyncTriggerBegin) {
				/* 状態：同期開始 */
				enableSignal = 0;
				enableSync = 0;
			} else if (mCount < SyncTriggerEnd) {
				/* 状態：トリガー出力 */
				enableSignal = 0;
				enableSync = 0;
				mIdxY = mOddEven;
				mIdxC = mOddEven * 2;
				trigger = mCount < SyncTriggerMid ? -0.9 : 0.9;
			} else {
				/* 状態：クロマ同期出力 */
				enableSignal = 0;
				if (SyncChromaBegin <= mCount && mCount < SyncChromaEnd) {
					enableSync = 1;
				} else {
					enableSync = 0;
				}
			}

			/* RGB -> YIQ */
			var r = pix[ptr];
			var g = pix[ptr + 1];
			var b = pix[ptr + 2];
			var y = (0.299 * r + 0.587 * g + 0.114 * b) * 0.5 / 255;
			var ci = (0.596 * r - 0.274 * g - 0.322 * b) * 0.333 / 255;
			var cq = (0.211 * r - 0.522 * g + 0.311 * b) * 0.333 / 255;
			/* トリガー */
			mT += (trigger - mT) * 0.5;
			/* 輝度振幅 */
			mY += (enableSignal * y - mY) * 0.5;
			/* クロマ信号振幅 */
			mCi += (enableSignal * ci - mCi) * 0.333;
			mCq += (enableSignal * cq - mCq) * 0.333;
			/* クロマ同期振幅 */
			mChromaSyncAmp += (enableSync * 0.2 - mChromaSyncAmp) * 0.333;
			/* 出力 */
			output = mT + Y[mIdxY] * mY + Re[mIdxC] * mCi + Im[mIdxC] * (mCq + mChromaSyncAmp);
			/* 輝度発振器 */
			mIdxY++;
			mIdxY -= mIdxY / 2 * 2;
			/* クロマ発振器 */
			mIdxC++;
			mIdxC -= mIdxC / 4 * 4;

			mCount++;
			if (0 < mEnableSync && SyncSamples < mCount) {
				mEnableSync = 0;
				mOddEven = (mOddEven + 1) % 2;
			}
			return 0 == mEnableSync;
		}

		public bool Decode(byte input, ref byte r, ref byte g, ref byte b) {
			// Y = |s0 + s2|
			// C = (s0 - s2)*2
			var s0 = (input - 128) / 128.0;
			var y = s0 + mS2;
			var c = (s0 - mS2) * 2;
			mS2 = mS1;
			mS1 = s0;

			var cre = c * Re[mIdxC];
			var cim = c * Im[mIdxC];
			mIdxC++;
			mIdxC -= mIdxC / 4 * 4;

			if (y < 0) {
				y *= -1;
			}

			//R = y + 0.956 * ci + 0.623 * cq;
			//G = y - 0.272 * ci - 0.648 * cq;
			//B = y - 1.105 * ci + 0.705 * cq;
			return true;
		}
	}
}
