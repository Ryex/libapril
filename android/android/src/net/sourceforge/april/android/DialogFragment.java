package net.sourceforge.april.android;

// version 3.3

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;

public class DialogFragment extends android.app.DialogFragment
{
	public DialogFragment()
	{
		super();
	}
	
	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState)
	{
		return DialogFactory.show();
	}
	
	@Override
	public void onCancel(DialogInterface dialog)
	{
		dialog.cancel();
		NativeInterface.AprilActivity.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onDialogCancel();
			}
		});
	}

}
