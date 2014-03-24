#ifndef BITMANIPULATION_H_
#define BITMANIPULATION_H_

// Extrait de http://www.ebyte.it/library/codesnippets/StansCppMacros_BitsAndFlags.html

#define mBits(x,m) ((x) & (m))
#define mBitsExcept(x,m,be) (((x) & (m)) & ~(be))
#define mTestBits(x,m,b) (mBits(x,m) == (b))
#define mTestBitsExcept(x,m,b,be) (mBitsExcept(x,m,be)==(b))

#define mBitsOn(lvx,by) ((lvx)|=(by))
#define mBitsOnExcept(lvx,by,be) ((lvx)|=(by)&~(be))
#define mBitsOff(lvx,bn) ((lvx)&= ~(bn))
#define mBitsOffExcept(lvx,bn,be) ((lvx)&= ~((bn)&~(be)))
#define mBitsToggle(lvx,b) ((lvx)^= (b))
#define mBitsToggleExcept(lvx,b,be) ((lvx)^= ((b)&~(be)))

#define mBitsOnOff(lvx,by,bn) {mBitsOff(lvx,bn);mBitsOn(lvx,by);}
#define mBitsOnOffExcept(lvx,by,bn,be) {mBitsOffExcept(lvx,bn,be);mBitsOnExcept(lvx,by,be);}


#endif /* BITMANIPULATION_H_ */