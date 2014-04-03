package net.sourceforge.april.android;

// version 3.3

import android.content.Context;
import android.graphics.PixelFormat;
import android.view.inputmethod.EditorInfo;
import android.view.MotionEvent;
import android.text.InputType;

public class GLSurfaceView extends android.opengl.GLSurfaceView
{
	protected net.sourceforge.april.android.Renderer renderer;
	
	public GLSurfaceView(Context context)
	{
		super(context);
		this.setEGLConfigChooser(8, 8, 8, 8, 0, 0);
		this.getHolder().setFormat(PixelFormat.RGBA_8888);
		this.renderer = new net.sourceforge.april.android.Renderer();
		this.setRenderer(this.renderer);
		// view has to be properly focusable to be able to process input
		this.setFocusable(true);
		this.setFocusableInTouchMode(true);
		this.setId(0x0513BEEF); // who doesn't love half a kg of beef?
	}
	
	@Override
	public void onWindowFocusChanged(final boolean focused)
	{
		if (focused)
		{
			this.requestFocus();
			this.requestFocusFromTouch();
			NativeInterface.updateKeyboard();
		}
		NativeInterface.AprilActivity.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onWindowFocusChanged(focused);
			}
		});
	}
	
	public boolean onTouchEvent(final MotionEvent event)
	{
		final int action = event.getAction();
		final int type;
		switch (action & MotionEvent.ACTION_MASK)
		{
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_POINTER_DOWN: // handles multi-touch
			type = 0;
			break;
		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_POINTER_UP: // handles multi-touch
			type = 1;
			break;
		case MotionEvent.ACTION_MOVE: // Android batches multitouch move events into a single move event
			type = 3;
			break;
		default:
			type = -1;
			break;
		}
		if (type >= 0)
		{
			this.queueEvent(new Runnable()
			{
				public void run()
				{
					final int pointerCount = event.getPointerCount();
					for (int i = 0; i < pointerCount; i++)
					{
						NativeInterface.onTouch(type, event.getX(i), event.getY(i), i);
					}
				}
			});
			return true;
		}
		return false;
	}
	
	@Override 
	public InputConnection onCreateInputConnection(EditorInfo outAttributes)  // required for creation of soft keyboard
	{ 
		outAttributes.actionId = EditorInfo.IME_ACTION_DONE;
		outAttributes.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI;
		if (NativeInterface.AprilActivity.OuyaKeyboardFix) // OUYA software keyboard doesn't appear unless TYPE_CLASS_TEXT is specified as input type
		{
			outAttributes.inputType = InputType.TYPE_CLASS_TEXT;
		}
		return new InputConnection(this);
	}
	
	public void swapBuffers()
	{
		this.renderer.swapBuffers();
	}
	
	@Override
	public boolean onCheckIsTextEditor() // required for creation of soft keyboard
	{
		return true;
	}
	
}

