package net.sourceforge.april.android;

// version 3.3

import android.app.AlertDialog;
import android.app.Dialog;
import android.os.Build;

import net.sourceforge.april.android.DialogListener.Cancel;
import net.sourceforge.april.android.DialogListener.Ok;
import net.sourceforge.april.android.DialogListener.No;
import net.sourceforge.april.android.DialogListener.Yes;

public class DialogFactory
{
	protected static AlertDialog.Builder dialogBuilder = null;
	protected static boolean useDialogFragment = true;
	protected static int dialogIndex = 0;
	
	public static void create(String title, String text, String ok, String yes, String no, String cancel, int iconId)
	{
		DialogFactory.dialogBuilder = new AlertDialog.Builder(NativeInterface.Activity);
		DialogFactory.dialogBuilder.setTitle(title != null ? title : "");
		DialogFactory.dialogBuilder.setMessage(text != null ? text : "");
		if (ok != null)
		{
			DialogFactory.dialogBuilder.setPositiveButton(ok, new Ok());
		}
		else
		{
			if (yes != null)
			{
				DialogFactory.dialogBuilder.setPositiveButton(yes, new Yes());
			}
			if (no != null)
			{
				DialogFactory.dialogBuilder.setNegativeButton(no, new No());
			}
		}
		if (cancel != null)
		{
			DialogFactory.dialogBuilder.setNeutralButton(cancel, new Cancel());
		}
		switch (iconId)
		{
		case 1:
			DialogFactory.dialogBuilder.setIcon(android.R.drawable.ic_dialog_info);
			break;
		case 2:
			DialogFactory.dialogBuilder.setIcon(android.R.drawable.ic_dialog_alert);
			break;
		default:
			break;
		}
		if (DialogFactory.useDialogFragment)
		{
			try
			{
				DialogFragment dialogFragment = new DialogFragment();
				dialogFragment.show(NativeInterface.Activity.getFragmentManager(), "april-dialog");
			}
			catch (java.lang.Throwable e)
			{
				android.util.Log.w("april", "This Android OS version does not support DialogFragment, defaulting to legacy Activity.showDialog().");
				DialogFactory.useDialogFragment = false;
			}
		}
		if (!DialogFactory.useDialogFragment)
		{
			NativeInterface.Activity.runOnUiThread(new Runnable()
			{
				public void run()
				{
					NativeInterface.Activity.showDialog(DialogFactory.dialogIndex);
					DialogFactory.dialogIndex++;
				}
			});
		}
	}
	
	public static Dialog show()
	{
		Dialog dialog = DialogFactory.dialogBuilder.create();
		dialog.setCancelable(true);
		dialog.setCanceledOnTouchOutside(true);
		return dialog;
	}
	
}
