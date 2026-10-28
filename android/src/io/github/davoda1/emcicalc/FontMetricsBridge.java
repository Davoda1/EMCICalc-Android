// SPDX-License-Identifier: MIT
package io.github.davoda1.emcicalc;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.util.TypedValue;

/** Publishes Android's actual nonlinear SP conversion to the Qt GUI thread. */
public final class FontMetricsBridge {
    private static Resources resources;
    private static long generation;
    private static float defaultSp;
    private FontMetricsBridge() {}
    // Called on the Activity thread after Qt's configuration handling. Accesses
    // are synchronized so no partially updated DisplayMetrics cross into JNI.
    public static synchronized void update(Context context) {
        resources = context.getResources();
        // Qt caches extracted styles by density, which can retain yesterday's
        // resolved pixel size after a font setting changes. Read the original
        // nominal textAppearance size from the current theme, not that cache.
        TypedValue appearance = new TypedValue();
        if (context.getTheme().resolveAttribute(android.R.attr.textAppearance, appearance, true)) {
            TypedArray attrs = context.obtainStyledAttributes(appearance.resourceId,
                    new int[] { android.R.attr.textSize });
            try {
                TypedValue size = attrs.peekValue(0);
                if (size != null && size.type == TypedValue.TYPE_DIMENSION) {
                    defaultSp = size.getComplexUnit() == TypedValue.COMPLEX_UNIT_SP
                            ? TypedValue.complexToFloat(size.data)
                            : TypedValue.deriveDimension(TypedValue.COMPLEX_UNIT_SP,
                                attrs.getDimension(0, 0), resources.getDisplayMetrics());
                }
            } finally { attrs.recycle(); }
        }
        ++generation;
    }
    public static synchronized float defaultSp() { return defaultSp; }
    public static synchronized long generation() { return generation; }
    public static synchronized float spToPx(float sp) {
        if (resources == null) return 0;
        return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP, sp,
                resources.getDisplayMetrics());
    }
    // A nominal SP at Android's standard font size is one DP. Do not divide
    // scaledDensity/fontScale: Android 14+ applies a nonlinear SP curve.
    // DP retains display-density/Display size changes while ignoring Font size.
    public static synchronized float dpToPx(float dp) {
        if (resources == null) return 0;
        return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dp,
                resources.getDisplayMetrics());
    }
    public static synchronized float pxToDp(float px) {
        if (resources == null) return 0;
        return TypedValue.deriveDimension(TypedValue.COMPLEX_UNIT_DIP, px,
                resources.getDisplayMetrics());
    }
    public static synchronized float pxToSp(float px) {
        if (resources == null) return 0;
        return TypedValue.deriveDimension(TypedValue.COMPLEX_UNIT_SP, px,
                resources.getDisplayMetrics());
    }
}
