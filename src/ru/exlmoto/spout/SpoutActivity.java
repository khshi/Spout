package ru.exlmoto.spout;

import ru.exlmoto.spout.SpoutLauncher.SpoutSettings;
import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Vibrator;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup.LayoutParams;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;

public class SpoutActivity extends Activity {

	private SpoutNativeSurface m_spoutNativeSurface;

	private static Vibrator m_vibrator;

	private static final String APP_TAG = "Spout_App";

	private static final int VIBRATION_DURATION = 50;

	private static boolean holdPushed = false;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		/* We like to be fullscreen */
		getWindow().setFlags(
				WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		requestWindowFeature(Window.FEATURE_NO_TITLE);

		super.onCreate(savedInstanceState);

		m_vibrator = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);

		m_spoutNativeSurface = new SpoutNativeSurface(this);
		setContentView(m_spoutNativeSurface);

		if (!SpoutSettings.s_DisableButtons) {
			float densityPixels = getResources().getDisplayMetrics().density;
			toDebug("PixelDensity: " + densityPixels);

			int padding = (int)(50 * densityPixels);
			toDebug("Padding: " + padding);

			LinearLayout.LayoutParams parametersBf =
					new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT,
							LayoutParams.WRAP_CONTENT);
			int leftF = (int)(20 * densityPixels);
			int topF = 0;
			int rightF = (int)(20 * densityPixels);
			int bottomF = (int)(10 * densityPixels);
			parametersBf.setMargins(leftF, topF, rightF, bottomF);

			LinearLayout.LayoutParams parametersbLR =
					new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT,
							LayoutParams.WRAP_CONTENT);
			int leftLR = 0;
			int topLR = 0;
			int rightLR = 0;
			int bottomLR = (int)(10 * densityPixels);
			parametersbLR.setMargins(leftLR, topLR, rightLR, bottomLR);


			final Button buttonFireHold = new Button(this);
			if (SpoutSettings.s_ShowButtons) {
				buttonFireHold.setBackgroundColor(Color.argb(100, 229, 82, 90));
			}
			buttonFireHold.setOnTouchListener(new OnTouchListener() {

				@Override
				public boolean onTouch(View v, MotionEvent event) {
					switch (event.getAction()) {
					case MotionEvent.ACTION_DOWN:
						holdPushed = !holdPushed;

						if (holdPushed) {
							SpoutNativeLibProxy.SpoutNativeKeyDown(SpoutNativeSurface.KEY_FIRE);
							if (SpoutSettings.s_ShowButtons) {
								v.setBackgroundColor(Color.argb(100, 142, 207, 106));
							}
						} else {
							SpoutNativeLibProxy.SpoutNativeKeyUp(SpoutNativeSurface.KEY_FIRE);
							if (SpoutSettings.s_ShowButtons) {
								v.setBackgroundColor(Color.argb(100, 229, 82, 90));
							}
						}

						v.setPressed(holdPushed);
						break;
						//				case MotionEvent.ACTION_UP:
						//					SpoutNativeLibProxy.SpoutNativeKeyUp(SpoutNativeSurface.KEY_FIRE);
						//					holdPushed = !holdPushed;
						//					v.setPressed(holdPushed);
						//					break;
					default:
						break;
					}
					return false;
				}

			});

			buttonFireHold.setText(R.string.HoldText);
			if (!SpoutSettings.s_ShowButtons) {
				buttonFireHold.setBackgroundColor(Color.argb(0, 255, 255, 255));
				buttonFireHold.setTextColor(Color.argb(75, 212, 207, 199));
			}
			buttonFireHold.setPadding(padding, padding, padding, padding);
			buttonFireHold.setLayoutParams(parametersBf);


			Button buttonFire = new Button(this);
			buttonFire.setOnTouchListener(new OnTouchListener() {

				@Override
				public boolean onTouch(View v, MotionEvent event) {
					switch (event.getAction()) {
					case MotionEvent.ACTION_DOWN:
						if (SpoutSettings.s_ShowButtons) {
							buttonFireHold.setBackgroundColor(Color.argb(100, 229, 82, 90));
						}
						SpoutNativeLibProxy.SpoutNativeKeyDown(SpoutNativeSurface.KEY_FIRE);
						break;
					case MotionEvent.ACTION_UP:
						SpoutNativeLibProxy.SpoutNativeKeyUp(SpoutNativeSurface.KEY_FIRE);
						break;
					default:
						break;
					}
					return false;
				}

			});
			buttonFire.setText(R.string.FireText);
			if (!SpoutSettings.s_ShowButtons) {
				buttonFire.setBackgroundColor(Color.argb(0, 255, 255, 255));
				buttonFire.setTextColor(Color.argb(75, 212, 207, 199));
			}
			buttonFire.setPadding(padding, padding, padding, padding);
			buttonFire.setLayoutParams(parametersBf);

			Button buttonLeft = new Button(this);
			buttonLeft.setOnTouchListener(new OnTouchListener() {

				@Override
				public boolean onTouch(View v, MotionEvent event) {
					switch (event.getAction()) {
					case MotionEvent.ACTION_DOWN:
						SpoutNativeLibProxy.SpoutNativeKeyDown(SpoutNativeSurface.KEY_LEFT);
						break;
					case MotionEvent.ACTION_UP:
						SpoutNativeLibProxy.SpoutNativeKeyUp(SpoutNativeSurface.KEY_LEFT);
						break;
					default:
						break;
					}
					return false;
				}

			});
			buttonLeft.setText(R.string.LeftText);
			if (!SpoutSettings.s_ShowButtons) {
				buttonLeft.setBackgroundColor(Color.argb(0, 255, 255, 255));
				buttonLeft.setTextColor(Color.argb(75, 212, 207, 199));
			}
			buttonLeft.setPadding(padding, padding, padding, padding);
			buttonLeft.setLayoutParams(parametersbLR);

			Button buttonRight = new Button(this);
			buttonRight.setOnTouchListener(new OnTouchListener() {

				@Override
				public boolean onTouch(View v, MotionEvent event) {
					switch (event.getAction()) {
					case MotionEvent.ACTION_DOWN:
						SpoutNativeLibProxy.SpoutNativeKeyDown(SpoutNativeSurface.KEY_RIGHT);
						break;
					case MotionEvent.ACTION_UP:
						SpoutNativeLibProxy.SpoutNativeKeyUp(SpoutNativeSurface.KEY_RIGHT);
						break;
					default:
						break;
					}
					return false;
				}

			});
			buttonRight.setText(R.string.RightText);
			if (!SpoutSettings.s_ShowButtons) {
				buttonRight.setBackgroundColor(Color.argb(0, 255, 255, 255));
				buttonRight.setTextColor(Color.argb(75, 212, 207, 199));
			}
			buttonRight.setPadding(padding, padding, padding, padding);

			buttonRight.setLayoutParams(parametersbLR);

			LinearLayout ll0 = new LinearLayout(this);
			ll0.addView(buttonFireHold);
			ll0.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.TOP);
			addContentView(ll0, new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT,
					LayoutParams.MATCH_PARENT));


			LinearLayout ll = new LinearLayout(this);

			//		ll.setAlpha(0);

			ll.addView(buttonLeft);
			ll.addView(buttonFire);
			ll.addView(buttonRight);
			ll.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.BOTTOM);

			addContentView(ll, new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT,
					LayoutParams.MATCH_PARENT));
		}
	}

	// JNI-method
	public static void setScores(int scoresH, int scoresS) {
		SpoutActivity.toDebug("--- From JNI!, a1: " + scoresH + " a2: " + scoresS);

		SpoutSettings.s_scoreHeight = scoresH;
		SpoutSettings.s_scoreScore = scoresS;
	}

	public static void toDebug(String s) {
		Log.d(APP_TAG, s);
	}

	public static void doVibrate() {
		m_vibrator.vibrate(VIBRATION_DURATION);
	}

	private void writeScoresToSharedPreferences() {
		toDebug("write scores... " + SpoutSettings.s_scoreHeight + " " + SpoutSettings.s_scoreScore);
		SharedPreferences settings = getSharedPreferences("ru.exlmoto.spout", MODE_PRIVATE);
		SharedPreferences.Editor editor = settings.edit();
		editor.putInt("s_scoreHeight", SpoutSettings.s_scoreHeight);
		editor.putInt("s_scoreScore", SpoutSettings.s_scoreScore);
		editor.commit();
	}

	@Override
	protected void onDestroy() {
		toDebug("Destroying...");
		//TODO: call score save method ?
		//m_spoutNativeSurface.onClose();
		super.onDestroy();
	}

	@Override
	public void onBackPressed() {
		toDebug("Back key pressed!, Exiting...");

		writeScoresToSharedPreferences();

		m_spoutNativeSurface.onPause();
		m_spoutNativeSurface.onClose();

		// super.onBackPressed();
		// Because we want drop all memory of library
		System.exit(0);
	}
}
