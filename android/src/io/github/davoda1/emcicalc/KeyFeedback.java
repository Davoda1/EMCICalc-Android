// SPDX-License-Identifier: MIT
package io.github.davoda1.emcicalc;

import android.app.Activity;
import android.app.Application;
import android.content.ComponentName;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.SoundPool;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.util.Log;
import android.view.HapticFeedbackConstants;
import android.view.View;
import android.view.Window;
import java.lang.ref.WeakReference;

/** App-owned key audio plus independent system haptics; all state lives on the UI thread. */
public final class KeyFeedback {
    private static final String TAG = "EMCICalcKeyFeedback";
    private static final Handler MAIN = new Handler(Looper.getMainLooper());
    private static final long MAX_PENDING_AGE_MS = 250;
    private static Application application;
    private static ComponentName ownerComponent;
    private static WeakReference<Activity> owner = new WeakReference<>(null);
    private static boolean foreground;
    private static boolean soundEnabled = true;
    private static SoundPool pool;
    private static int clickId;
    private static boolean loaded;
    private static long pendingClickAt = -1;
    private static long nextLoadAttemptAt;

    private KeyFeedback() {}

    private static final Application.ActivityLifecycleCallbacks LIFECYCLE =
        new Application.ActivityLifecycleCallbacks() {
            @Override public void onActivityCreated(Activity activity, Bundle savedState) {}
            @Override public void onActivityStarted(Activity activity) {}
            @Override public void onActivityResumed(Activity activity) {
                if (!activity.getComponentName().equals(ownerComponent)) return;
                owner = new WeakReference<>(activity);
                foreground = true;
                activity.setVolumeControlStream(AudioManager.STREAM_MUSIC);
                ensurePool(); // Decode before the first key after returning to the app.
            }
            @Override public void onActivityPaused(Activity activity) {
                if (owner.get() != activity) return;
                foreground = false;
                releasePool(); // Never replay a queued or paused click in the background.
            }
            @Override public void onActivityStopped(Activity activity) {}
            @Override public void onActivitySaveInstanceState(Activity activity, Bundle outState) {}
            @Override public void onActivityDestroyed(Activity activity) {
                if (owner.get() != activity) return;
                foreground = false;
                owner.clear();
                releasePool();
            }
        };

    private static void onMain(Runnable action) {
        if (Looper.myLooper() == Looper.getMainLooper()) action.run();
        else MAIN.post(action);
    }

    /** Called after loading app settings; starts asynchronous decoding before any key is tapped. */
    public static void initialize(Activity activity, final boolean sound) {
        final WeakReference<Activity> reference = new WeakReference<>(activity);
        onMain(() -> {
            final Activity current = reference.get();
            if (!usable(current)) return;
            attach(current);
            soundEnabled = sound;
            if (soundEnabled) ensurePool();
            else releasePool();
        });
    }

    /** Cancels pending audio immediately when the independently saved sound setting is disabled. */
    public static void setSoundEnabled(final boolean enabled) {
        onMain(() -> {
            soundEnabled = enabled;
            if (enabled) ensurePool();
            else releasePool();
        });
    }

    public static void shutdown() {
        onMain(() -> {
            foreground = false;
            releasePool();
            owner.clear();
            ownerComponent = null;
            if (application != null) application.unregisterActivityLifecycleCallbacks(LIFECYCLE);
            application = null;
        });
    }

    public static void perform(Activity activity, final boolean sound, final boolean haptic) {
        if (activity == null || (!sound && !haptic)) return;
        final WeakReference<Activity> reference = new WeakReference<>(activity);
        onMain(() -> {
            final Activity current = reference.get();
            if (!usable(current)) return;
            if (application == null) attach(current); // Defensive fallback for a recreated Qt bridge.
            final View view = focusedView(current);
            if (owner.get() != current || !foreground || view == null) return;
            // Haptics intentionally keep Android's own preference and accessibility policy.
            if (haptic) view.performHapticFeedback(HapticFeedbackConstants.KEYBOARD_TAP);
            if (!sound || !soundEnabled) return;
            ensurePool();
            if (loaded) playClick();
            else if (pool != null) pendingClickAt = SystemClock.uptimeMillis();
        });
    }

    private static boolean usable(Activity activity) {
        return activity != null && !activity.isFinishing() && !activity.isDestroyed();
    }

    private static View focusedView(Activity activity) {
        if (!usable(activity)) return null;
        final Window window = activity.getWindow();
        if (window == null) return null;
        final View view = window.getDecorView();
        return view != null && view.isAttachedToWindow() && view.hasWindowFocus() ? view : null;
    }

    private static void attach(Activity activity) {
        final Application currentApplication = activity.getApplication();
        if (application != currentApplication) {
            if (application != null) application.unregisterActivityLifecycleCallbacks(LIFECYCLE);
            releasePool();
            application = currentApplication;
            application.registerActivityLifecycleCallbacks(LIFECYCLE);
        }
        ownerComponent = activity.getComponentName();
        owner = new WeakReference<>(activity);
        foreground = true;
        // Volume buttons control this app's click volume, not the unrelated system-touch volume.
        activity.setVolumeControlStream(AudioManager.STREAM_MUSIC);
    }

    private static void ensurePool() {
        if (!soundEnabled || !foreground || application == null || pool != null
                || SystemClock.uptimeMillis() < nextLoadAttemptAt) return;
        try {
            // This is a bundled app sound on media volume. It does not call View.playSoundEffect,
            // depend on the system touch-sounds switch, alter volume, or force audibility.
            final AudioAttributes attributes = new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_MEDIA)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build();
            final SoundPool newPool = new SoundPool.Builder()
                .setMaxStreams(4)
                .setAudioAttributes(attributes)
                .build();
            pool = newPool;
            newPool.setOnLoadCompleteListener((completedPool, sampleId, status) -> onMain(() -> {
                if (pool != completedPool || sampleId != clickId) return;
                if (status != 0) {
                    Log.w(TAG, "Could not decode the bundled calculator click: " + status);
                    releasePool();
                    nextLoadAttemptAt = SystemClock.uptimeMillis() + 1000;
                    return;
                }
                loaded = true;
                final long age = SystemClock.uptimeMillis() - pendingClickAt;
                final boolean playPending = pendingClickAt >= 0 && age <= MAX_PENDING_AGE_MS;
                pendingClickAt = -1;
                // One recent tap may wait for preload; never emit a burst of stale queued keys.
                if (playPending && soundEnabled && foreground && focusedView(owner.get()) != null)
                    playClick();
            }));
            clickId = newPool.load(application, R.raw.emcicalc_key_click, 1);
            if (clickId == 0) throw new IllegalStateException("Bundled click could not be loaded");
        } catch (RuntimeException error) {
            Log.w(TAG, "Calculator key audio unavailable", error);
            releasePool();
            nextLoadAttemptAt = SystemClock.uptimeMillis() + 1000;
        }
    }

    private static void playClick() {
        if (pool == null || !loaded) return;
        // A single short sample, no loop; SoundPool mixes rapid taps without blocking the UI.
        try {
            if (pool.play(clickId, 1.0f, 1.0f, 1, 0, 1.0f) != 0) return;
            Log.w(TAG, "Calculator click playback failed; retrying on a later key");
        } catch (RuntimeException error) {
            Log.w(TAG, "Calculator click playback unavailable", error);
        }
        releasePool();
        nextLoadAttemptAt = SystemClock.uptimeMillis() + 250;
    }

    private static void releasePool() {
        final SoundPool oldPool = pool;
        pool = null;
        clickId = 0;
        loaded = false;
        pendingClickAt = -1;
        if (oldPool != null) {
            oldPool.setOnLoadCompleteListener(null);
            oldPool.release();
        }
    }
}
