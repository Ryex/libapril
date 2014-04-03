package net.sourceforge.april.android;

// version 3.3

import android.view.inputmethod.BaseInputConnection;
import android.view.KeyEvent;
import android.view.View;

class InputConnection extends BaseInputConnection
{
	private final KeyEvent delKeyDownEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL);
	private final KeyEvent delKeyUpEvent = new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_DEL);
	
	public InputConnection(View view)
	{
		super(view, false);
		this.setSelection(0, 0);
	}
	
	@Override
	public boolean deleteSurroundingText(int leftLength, int rightLength)
	{
		// Android SDK 16+ doesn't send key events for backspace but calls this method
		NativeInterface.Activity.onKeyDown(KeyEvent.KEYCODE_DEL, this.delKeyDownEvent);
		NativeInterface.Activity.onKeyUp(KeyEvent.KEYCODE_DEL, this.delKeyUpEvent);
		return super.deleteSurroundingText(leftLength, rightLength);
	}
	
}
