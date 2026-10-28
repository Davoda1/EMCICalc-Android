// SPDX-License-Identifier: MIT
package io.github.davoda1.emcicalc;

import android.content.res.Configuration;
import android.os.Bundle;
import org.qtproject.qt.android.bindings.QtActivity;

/** Retains Qt's lifecycle and publishes font/density changes without a restart. */
public final class EMCICalcActivity extends QtActivity {
    @Override public void setTheme(int resourceId) {
        super.setTheme(resourceId);
        // QtActivityBase selects its DayNight theme inside super.onCreate,
        // before starting native code. Publish that actual theme immediately.
        FontMetricsBridge.update(this);
    }
    @Override public void onCreate(Bundle state) {
        // Qt starts its native thread during super.onCreate(). Publish first.
        FontMetricsBridge.update(this);
        super.onCreate(state);
    }
    @Override public void onConfigurationChanged(Configuration configuration) {
        super.onConfigurationChanged(configuration);
        FontMetricsBridge.update(this);
    }
    @Override public void onResume() {
        super.onResume();
        FontMetricsBridge.update(this);
    }
}
