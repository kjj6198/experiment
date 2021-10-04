#include "LedControl.h"

class LEDDisplay {
	public :
		int i;
		int j;
		LedControl lc;
		LEDDisplay(int dataPin, int clkPin, int csPin);
		~LEDDisplay();
		void init();
		// render all
		void render();
		// TODO: find the diff and only set those bits
		// will that be faster?
		void diff();
		void fill(int row, int col, int val);
		void zeros();
		void blink();
};