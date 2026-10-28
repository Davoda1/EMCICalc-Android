#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run actual KeyFeedback.java with a deterministic host Android lifecycle/audio adapter.

This tests scheduling and state transitions, not a phone speaker, Android codec,
audio routing or haptic hardware. Compile against android.jar separately as well.
"""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

STUBS = {
    'android/content/Context.java': 'package android.content; public class Context {}',
    'android/content/ComponentName.java': '''package android.content;
public final class ComponentName {
 private final String name; public ComponentName(String value){name=value;}
 @Override public boolean equals(Object other){return other instanceof ComponentName && name.equals(((ComponentName)other).name);}
 @Override public int hashCode(){return name.hashCode();}
}''',
    'android/os/Bundle.java': 'package android.os; public class Bundle {}',
    'android/os/Looper.java': '''package android.os; public final class Looper {
 private static final Looper MAIN=new Looper(); public static boolean onMain=true;
 public static Looper getMainLooper(){return MAIN;} public static Looper myLooper(){return onMain?MAIN:null;}
}''',
    'android/os/Handler.java': '''package android.os; import java.util.ArrayList;
public final class Handler {
 private static final ArrayList<Runnable> queue=new ArrayList<>(); public Handler(Looper loop){}
 public boolean post(Runnable runnable){queue.add(runnable);return true;}
 public static void drain(){Looper.onMain=true; while(!queue.isEmpty())queue.remove(0).run();}
}''',
    'android/os/SystemClock.java': '''package android.os; public final class SystemClock {
 public static long now=1000; public static long uptimeMillis(){return now;}
}''',
    'android/util/Log.java': '''package android.util; public final class Log {
 public static int warnings; public static int w(String tag,String message){warnings++;return 0;}
 public static int w(String tag,String message,Throwable error){warnings++;return 0;}
}''',
    'android/view/HapticFeedbackConstants.java': 'package android.view; public final class HapticFeedbackConstants {public static final int KEYBOARD_TAP=3;}',
    'android/view/View.java': '''package android.view; public class View {
 public boolean attached=true, focused=true; public int haptics;
 public boolean isAttachedToWindow(){return attached;} public boolean hasWindowFocus(){return focused;}
 public boolean performHapticFeedback(int kind){if(kind!=HapticFeedbackConstants.KEYBOARD_TAP)throw new AssertionError();haptics++;return true;}
}''',
    'android/view/Window.java': '''package android.view; public final class Window {
 public final View view=new View(); public View getDecorView(){return view;}
}''',
    'android/app/Application.java': '''package android.app;
import android.content.Context; import android.os.Bundle;
public final class Application extends Context {
 public ActivityLifecycleCallbacks callbacks; public int registrations,unregistrations;
 public void registerActivityLifecycleCallbacks(ActivityLifecycleCallbacks value){callbacks=value;registrations++;}
 public void unregisterActivityLifecycleCallbacks(ActivityLifecycleCallbacks value){if(callbacks==value)callbacks=null;unregistrations++;}
 public interface ActivityLifecycleCallbacks {
  void onActivityCreated(Activity activity,Bundle savedState); void onActivityStarted(Activity activity);
  void onActivityResumed(Activity activity); void onActivityPaused(Activity activity);
  void onActivityStopped(Activity activity); void onActivitySaveInstanceState(Activity activity,Bundle state);
  void onActivityDestroyed(Activity activity);
 }
}''',
    'android/app/Activity.java': '''package android.app;
import android.content.ComponentName; import android.view.Window;
public final class Activity {
 public final Window window=new Window(); public final Application app;
 public boolean finishing,destroyed; public int volumeStream=-1;
 public Activity(Application value){app=value;} public boolean isFinishing(){return finishing;}
 public boolean isDestroyed(){return destroyed;} public Window getWindow(){return window;}
 public Application getApplication(){return app;} public ComponentName getComponentName(){return new ComponentName("EMCICalc");}
 public void setVolumeControlStream(int stream){volumeStream=stream;}
}''',
    'android/media/AudioManager.java': 'package android.media; public final class AudioManager {public static final int STREAM_MUSIC=3;}',
    'android/media/AudioAttributes.java': '''package android.media; public final class AudioAttributes {
 public static final int USAGE_MEDIA=1,CONTENT_TYPE_SONIFICATION=4; public int usage,content;
 public static final class Builder {
  private final AudioAttributes attributes=new AudioAttributes();
  public Builder setUsage(int value){attributes.usage=value;return this;}
  public Builder setContentType(int value){attributes.content=value;return this;}
  public AudioAttributes build(){return attributes;}
 }
}''',
    'android/media/SoundPool.java': '''package android.media; import android.content.Context;
import java.util.ArrayList;
public final class SoundPool {
 public static final ArrayList<SoundPool> instances=new ArrayList<>();
 public static boolean failBuild,failPlay;
 public boolean released; public int plays,loads,maxStreams; public AudioAttributes attributes;
 public OnLoadCompleteListener listener,lastListener;
 public interface OnLoadCompleteListener {void onLoadComplete(SoundPool pool,int sampleId,int status);}
 public static final class Builder {
  private int max; private AudioAttributes attributes;
  public Builder setMaxStreams(int value){max=value;return this;}
  public Builder setAudioAttributes(AudioAttributes value){attributes=value;return this;}
  public SoundPool build(){if(failBuild)throw new IllegalStateException("test codec failure");
   SoundPool pool=new SoundPool();pool.maxStreams=max;pool.attributes=attributes;instances.add(pool);return pool;}
 }
 public void setOnLoadCompleteListener(OnLoadCompleteListener value){listener=value;if(value!=null)lastListener=value;}
 public int load(Context context,int resource,int priority){loads++;if(resource!=7)throw new AssertionError();return 1;}
 public int play(int sample,float left,float right,int priority,int loop,float rate){
  if(released||sample!=1||left!=1||right!=1||loop!=0||rate!=1)throw new AssertionError();return failPlay?0:++plays;}
 public void release(){if(released)throw new AssertionError("double release");released=true;}
 public void complete(int status){lastListener.onLoadComplete(this,1,status);}
 public static SoundPool latest(){return instances.get(instances.size()-1);}
}''',
    'io/github/davoda1/emcicalc/R.java': 'package io.github.davoda1.emcicalc; public final class R {public static final class raw {public static final int emcicalc_key_click=7;}}',
    'io/github/davoda1/emcicalc/KeyFeedbackTest.java': '''package io.github.davoda1.emcicalc;
import android.app.*; import android.media.*; import android.os.*;
public final class KeyFeedbackTest {
 private static int checks;
 private static void check(boolean condition,String name){if(!condition)throw new AssertionError(name);checks++;System.out.println("PASS: "+name);}
 public static void main(String[] args){
  Application app=new Application();Activity a=new Activity(app);
  KeyFeedback.initialize(a,false);KeyFeedback.perform(a,false,true);
  check(SoundPool.instances.isEmpty()&&a.window.view.haptics==1,"sound OFF, vibration ON is independent");
  KeyFeedback.setSoundEnabled(true);SoundPool first=SoundPool.latest();
  check(first.loads==1&&first.plays==0&&a.volumeStream==AudioManager.STREAM_MUSIC,"preload before first key and media volume control");
  check(first.attributes.usage==AudioAttributes.USAGE_MEDIA&&first.attributes.content==AudioAttributes.CONTENT_TYPE_SONIFICATION&&first.maxStreams==4,"media click attributes and bounded rapid-tap mixing");
  KeyFeedback.perform(a,true,false);check(first.plays==0,"undecoded sample is not played prematurely");
  first.complete(0);check(first.plays==1&&a.window.view.haptics==1,"first tap survives asynchronous preload without vibration");
  for(int i=0;i<20;i++)KeyFeedback.perform(a,true,false);
  check(first.plays==21,"rapid accepted keys play once each without repeat or loop");
  KeyFeedback.perform(a,false,false);check(first.plays==21&&a.window.view.haptics==1,"both feedback switches OFF");
  a.window.view.focused=false;KeyFeedback.perform(a,true,true);
  check(first.plays==21&&a.window.view.haptics==1,"unfocused window emits neither audio nor vibration");a.window.view.focused=true;
  app.callbacks.onActivityPaused(a);first.complete(0);
  check(first.released&&first.plays==21,"pause releases audio and ignores late decode completion");
  KeyFeedback.perform(a,true,true);check(first.plays==21&&a.window.view.haptics==1,"paused activity emits no feedback");
  app.callbacks.onActivityResumed(a);SoundPool second=SoundPool.latest();second.complete(0);
  check(second!=first&&second.plays==0,"resume preloads a new pool without replaying old clicks");
  KeyFeedback.perform(a,true,true);check(second.plays==1&&a.window.view.haptics==2,"sound and vibration ON together after resume");
  KeyFeedback.setSoundEnabled(false);KeyFeedback.setSoundEnabled(true);SoundPool third=SoundPool.latest();
  KeyFeedback.perform(a,true,false);KeyFeedback.setSoundEnabled(false);third.complete(0);
  check(third.released&&third.plays==0,"turning sound OFF cancels a pending first click");
  KeyFeedback.setSoundEnabled(true);SoundPool fourth=SoundPool.latest();
  for(int i=0;i<10;i++)KeyFeedback.perform(a,true,false);fourth.complete(0);
  check(fourth.plays==1,"rapid taps while decoding do not produce a delayed burst");
  KeyFeedback.setSoundEnabled(false);KeyFeedback.setSoundEnabled(true);SoundPool fifth=SoundPool.latest();
  KeyFeedback.perform(a,true,false);SystemClock.now+=251;fifth.complete(0);
  check(fifth.plays==0,"stale pending feedback expires instead of surprising the user later");
  Looper.onMain=false;KeyFeedback.perform(a,true,true);Looper.onMain=true;app.callbacks.onActivityPaused(a);Handler.drain();
  check(fifth.released&&fifth.plays==0&&a.window.view.haptics==2,"queued Qt-thread tap is cancelled when the activity pauses");
  a.destroyed=true;app.callbacks.onActivityDestroyed(a);Activity replacement=new Activity(app);
  app.callbacks.onActivityResumed(replacement);SoundPool sixth=SoundPool.latest();sixth.complete(0);
  KeyFeedback.perform(a,true,true);KeyFeedback.perform(replacement,true,false);
  check(sixth.plays==1&&a.window.view.haptics==2&&app.registrations==1,"activity recreation uses new owner and ignores destroyed owner");
  KeyFeedback.setSoundEnabled(false);KeyFeedback.setSoundEnabled(true);SoundPool seventh=SoundPool.latest();seventh.complete(1);
  KeyFeedback.perform(replacement,true,true);
  check(seventh.released&&replacement.window.view.haptics==1,"decode failure cannot disable independent vibration");
  SystemClock.now+=1001;SoundPool.failBuild=true;KeyFeedback.perform(replacement,true,true);
  check(replacement.window.view.haptics==2,"audio allocation failure is contained without losing key haptics");
  SoundPool.failBuild=false;SystemClock.now+=1001;KeyFeedback.perform(replacement,true,false);SoundPool eighth=SoundPool.latest();eighth.complete(0);
  check(eighth.plays==1,"audio recovers on a later tap after a transient backend failure");
  SoundPool.failPlay=true;KeyFeedback.perform(replacement,true,true);
  check(eighth.released&&replacement.window.view.haptics==3,"playback failure releases broken backend without losing haptics");
  SoundPool.failPlay=false;SystemClock.now+=251;KeyFeedback.perform(replacement,true,false);SoundPool ninth=SoundPool.latest();ninth.complete(0);
  check(ninth.plays==1,"playback backend can recover on a later key");
  KeyFeedback.shutdown();ninth.complete(0);
  check(ninth.released&&app.callbacks==null&&app.unregistrations==1,"shutdown releases native audio and lifecycle registration");
  System.out.println("PASS: "+checks+" lifecycle/scheduling checks (host adapter, not device audio)");
 }
}''',
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--javac', default=shutil.which('javac'))
    parser.add_argument('--java', default=shutil.which('java'))
    args = parser.parse_args()
    if not args.java or not args.javac:
        raise SystemExit('Java JDK required; pass --javac and --java or add them to PATH')
    root = Path(__file__).resolve().parents[1]
    source = root / 'android/src/io/github/davoda1/emcicalc/KeyFeedback.java'
    with tempfile.TemporaryDirectory(prefix='emcicalc-keyfeedback-') as directory:
        temporary = Path(directory)
        for name, content in STUBS.items():
            path = temporary / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content + '\n')
        classes = temporary / 'classes'
        subprocess.run([args.javac, '--release', '8', '-Xlint:all', '-d', str(classes),
                        str(source), *(str(temporary / name) for name in STUBS)], check=True)
        subprocess.run([args.java, '-cp', str(classes), 'io.github.davoda1.emcicalc.KeyFeedbackTest'], check=True)


if __name__ == '__main__':
    main()
