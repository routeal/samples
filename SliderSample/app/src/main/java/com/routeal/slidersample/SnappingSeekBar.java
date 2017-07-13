package com.routeal.slidersample;

//package com.tobishiba.snappingseekbar.library.views;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.support.v4.content.res.ResourcesCompat;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.animation.Animation;
import android.view.animation.Transformation;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

class ProgressBarAnimation extends Animation {
    private ProgressBar mProgressBar;
    private float mFrom;
    private float mTo;

    public ProgressBarAnimation(final ProgressBar progressBar, final float from, final float to) {
        super();
        this.mProgressBar = progressBar;
        this.mFrom = from;
        this.mTo = to;
    }

    @Override
    protected void applyTransformation(final float interpolatedTime, final Transformation t) {
        super.applyTransformation(interpolatedTime, t);
        float value = mFrom + (mTo - mFrom) * interpolatedTime;
        mProgressBar.setProgress((int) value);
    }
}

class ReversedSeekBar extends SeekBar {

    public ReversedSeekBar(Context context) {
        super(context);
    }

    public ReversedSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ReversedSeekBar(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        float px = this.getWidth() / 2.0f;
        float py = this.getHeight() / 2.0f;

        canvas.scale(-1, 1, px, py);

        super.onDraw(canvas);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        event.setLocation(this.getWidth() - event.getX(), event.getY());

        return super.onTouchEvent(event);
    }
}

/**
 * User: tobiasbuchholz
 * Date: 28.07.14 | Time: 14:18
 */
public class SnappingSeekBar extends RelativeLayout implements SeekBar.OnSeekBarChangeListener {
    public static final int NOT_INITIALIZED_THUMB_POSITION = -1;
    private Context mContext;
    private SeekBar mSeekBar;
    private int mItemsAmount;
    private int mFromProgress;
    private int mThumbPosition = NOT_INITIALIZED_THUMB_POSITION;
    private int mToProgress;
    private String[] mItems = new String[0];
    private int mProgressDrawableId;
    private int mThumbDrawableId;
    private int mIndicatorDrawableId;
    private int mProgressColor;
    private int mIndicatorColor;
    private int mThumbnailColor;
    private int mTextIndicatorColor;
    private float mTextIndicatorTopMargin;
    private int mTextStyleId;
    private float mTextSize;
    private float mIndicatorSize;
    private float mDensity;
    private Drawable mProgressDrawable;
    private Drawable mThumbDrawable;
    private OnItemSelectionListener mOnItemSelectionListener;

    public SnappingSeekBar(final Context context) {
        super(context);
        mContext = context;
        initDensity();
        initDefaultValues();
        initViewsAfterLayoutPrepared();
    }

    private void initDensity() {
        mDensity = mContext.getResources().getDisplayMetrics().density;
    }

    private void initDefaultValues() {
        mProgressDrawableId = R.drawable.seekbar_scrubber_progress_horizontal_holo_light;
        mThumbDrawableId = R.drawable.seekbar_scrubber_control_selector_holo_light;
        mIndicatorDrawableId = R.drawable.circle_background;
        mProgressColor = Color.WHITE;
        mIndicatorColor = Color.WHITE;
        mThumbnailColor = Color.WHITE;
        mTextIndicatorColor = Color.WHITE;
        mTextIndicatorTopMargin = 35 * mDensity;
        mTextSize = 12 * mDensity;
        mIndicatorSize = 11.3f * mDensity;
    }

    private void initViewsAfterLayoutPrepared() {
        waitForLayoutPrepared(this, new LayoutPreparedListener() {
            @Override
            public void onLayoutPrepared(final View preparedView) {
                initViews();
            }
        });
    }

    public SnappingSeekBar(final Context context, final AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        initDensity();
        handleAttributeSet(attrs);
        initViews();
    }

    private void handleAttributeSet(final AttributeSet attrs) {
        final TypedArray typedArray = mContext.getTheme().obtainStyledAttributes(attrs, R.styleable.SnappingSeekBar, 0, 0);
        try {
            initDrawables(typedArray);
            initItems(typedArray);
            initIndicatorAttributes(typedArray);
            initTextAttributes(typedArray);
            initColors(typedArray);
        } finally {
            typedArray.recycle();
        }
    }

    private void initDrawables(final TypedArray typedArray) {
        mProgressDrawableId = typedArray.getResourceId(R.styleable.SnappingSeekBar_progressDrawable, R.drawable.seekbar_scrubber_progress_horizontal_holo_light);
        mThumbDrawableId = typedArray.getResourceId(R.styleable.SnappingSeekBar_thumb, R.drawable.seekbar_scrubber_control_selector_holo_light);
        mIndicatorDrawableId = typedArray.getResourceId(R.styleable.SnappingSeekBar_indicatorDrawable, R.drawable.circle_background);
    }

    private void initItems(final TypedArray typedArray) {
        final int itemsArrayId = typedArray.getResourceId(R.styleable.SnappingSeekBar_itemsArrayId, 0);
        if (itemsArrayId > 0) {
            setItems(itemsArrayId);
        } else {
            setItemsAmount(typedArray.getInteger(R.styleable.SnappingSeekBar_itemsAmount, 10));
        }
    }

    public void setItems(final int itemsArrayId) {
        setItems(mContext.getResources().getStringArray(itemsArrayId));
    }

    public void setItems(final String[] items) {
        if (items.length > 1) {
            mItems = items;
            mItemsAmount = mItems.length;
        } else {
            throw new IllegalStateException("SnappingSeekBar has to contain at least 2 items");
        }
    }

    public void setItemsAmount(final int itemsAmount) {
        if (itemsAmount > 1) {
            mItemsAmount = itemsAmount;
        } else {
            throw new IllegalStateException("SnappingSeekBar has to contain at least 2 items");
        }
    }

    private void initIndicatorAttributes(final TypedArray typedArray) {
        mIndicatorSize = typedArray.getDimension(R.styleable.SnappingSeekBar_indicatorSize, 11.3f * mDensity);
    }

    private void initTextAttributes(final TypedArray typedArray) {
        mTextIndicatorTopMargin = typedArray.getDimension(R.styleable.SnappingSeekBar_textIndicatorTopMargin, 35 * mDensity);
        mTextStyleId = typedArray.getResourceId(R.styleable.SnappingSeekBar_textStyle, 0);
        mTextSize = typedArray.getDimension(R.styleable.SnappingSeekBar_textSize, 12 * mDensity);
    }

    private void initColors(final TypedArray typedArray) {
        mProgressColor = typedArray.getColor(R.styleable.SnappingSeekBar_progressColor, Color.WHITE);
        mIndicatorColor = typedArray.getColor(R.styleable.SnappingSeekBar_indicatorColor, Color.WHITE);
        mThumbnailColor = typedArray.getColor(R.styleable.SnappingSeekBar_thumbnailColor, Color.WHITE);
        mTextIndicatorColor = typedArray.getColor(R.styleable.SnappingSeekBar_textIndicatorColor, Color.WHITE);
    }

    public void initViews() {
        initSeekBar();
        initIndicators();
    }

    private void initSeekBar() {
        final LayoutParams params = new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        mSeekBar = new ReversedSeekBar(mContext);
        mSeekBar.setOnSeekBarChangeListener(this);
        mSeekBar.setLayoutParams(params);
        setDrawablesToSeekBar();
        addView(mSeekBar, params);
    }

    private void setDrawablesToSeekBar() {
        final Resources resources = getResources();
        mProgressDrawable = ResourcesCompat.getDrawable(resources, mProgressDrawableId, null);
        mThumbDrawable = ResourcesCompat.getDrawable(resources, mThumbDrawableId, null);
        setColor(mProgressDrawable, mProgressColor);
        setColor(mThumbDrawable, mThumbnailColor);
        mSeekBar.setProgressDrawable(mProgressDrawable);
        mSeekBar.setThumb(mThumbDrawable);
        final int thumbnailWidth = mThumbDrawable.getIntrinsicWidth();
        mSeekBar.setPadding(thumbnailWidth / 2, 0, thumbnailWidth / 2, 0);
    }

    private void initIndicators() {
        waitForLayoutPrepared(mSeekBar, new LayoutPreparedListener() {
            @Override
            public void onLayoutPrepared(final View preparedView) {
                final int seekBarWidth = preparedView.getWidth();
                initIndicators(seekBarWidth);
            }
        });
    }

    private void initIndicators(final int seekBarWidth) {
        for (int i = 0; i < mItemsAmount; i++) {
            addCircleIndicator(seekBarWidth, i);
            addTextIndicatorIfNeeded(seekBarWidth, i);
        }
    }

    private void addCircleIndicator(final int seekBarWidth, final int index) {
        final int thumbnailWidth = mThumbDrawable.getIntrinsicWidth();
        final int sectionFactor = 100 / (mItemsAmount - 1);
        final float seekBarWidthWithoutThumbOffset = seekBarWidth - thumbnailWidth;
        final LayoutParams indicatorParams = new LayoutParams((int) mIndicatorSize, (int) mIndicatorSize);
        final View indicator = new View(mContext);
        indicator.setBackgroundResource(mIndicatorDrawableId);
        setColor(indicator.getBackground(), mIndicatorColor);
        indicatorParams.leftMargin = (int) (seekBarWidthWithoutThumbOffset / 100 * index * sectionFactor + thumbnailWidth / 2 - mIndicatorSize / 2);
        indicatorParams.topMargin = mThumbDrawable.getIntrinsicHeight() / 2 - (int) (mIndicatorSize / 2);
        addView(indicator, indicatorParams);
    }

    private void addTextIndicatorIfNeeded(final int completeSeekBarWidth, final int index) {
        if (mItems.length == mItemsAmount) {
            addTextIndicator(completeSeekBarWidth, index);
        }
    }

    private void addTextIndicator(final int completeSeekBarWidth, final int index) {
        final int thumbnailWidth = mThumbDrawable.getIntrinsicWidth();
        final int sectionFactor = 100 / (mItemsAmount - 1);
        final float seekBarWidthWithoutThumbOffset = completeSeekBarWidth - thumbnailWidth;
        final LayoutParams textParams = new LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        final TextView textIndicator = new TextView(mContext);
        final int numberLeftMargin = (int) (seekBarWidthWithoutThumbOffset / 100 * index * sectionFactor + thumbnailWidth / 2);
        textIndicator.setText(mItems[index]);
        textIndicator.setTextSize(mTextSize / mDensity);
        textIndicator.setTextColor(mTextIndicatorColor);
        if (Build.VERSION.SDK_INT < 23) {
            textIndicator.setTextAppearance(mContext, mTextStyleId);
        } else {
            textIndicator.setTextAppearance(mTextStyleId);
        }
        textParams.topMargin = (int) mTextIndicatorTopMargin;
        addView(textIndicator, textParams);
        waitForLayoutPrepared(textIndicator, createTextIndicatorLayoutPreparedListener(numberLeftMargin));
    }

    private LayoutPreparedListener createTextIndicatorLayoutPreparedListener(final int numberLeftMargin) {
        return new LayoutPreparedListener() {
            @Override
            public void onLayoutPrepared(final View preparedView) {
                final int layoutWidth = getWidth() - getPaddingRight();
                final int viewWidth = preparedView.getWidth();
                final int leftMargin = numberLeftMargin - viewWidth / 2;
                final int paddingLeft = getPaddingLeft();
                final int finalMargin = leftMargin < paddingLeft ? paddingLeft : leftMargin + viewWidth > layoutWidth ? layoutWidth - viewWidth : leftMargin;
                setLeftMargin(preparedView, finalMargin);
            }
        };
    }

    @Override
    public void onProgressChanged(final SeekBar seekBar, final int progress, final boolean fromUser) {
        mToProgress = progress;
        initThumbPosition(progress, fromUser);
        handleSetFromProgress(progress);
    }

    private void initThumbPosition(final int progress, final boolean fromUser) {
        if (mThumbPosition == NOT_INITIALIZED_THUMB_POSITION && fromUser) {
            mThumbPosition = progress;
        }
    }

    private void handleSetFromProgress(final int progress) {
        final int slidingDelta = progress - mThumbPosition;
        if (slidingDelta > 1 || slidingDelta < -1) {
            mFromProgress = progress;
        }
    }

    private void handleSnapToClosestValue() {
        final float sectionLength = 100 / (mItemsAmount - 1);
        final int selectedSection = (int) ((mToProgress / sectionLength) + 0.5);
        final int valueToSnap = (int) (selectedSection * sectionLength);
        animateProgressBar(valueToSnap);
        invokeItemSelected(selectedSection);
    }

    private void animateProgressBar(final int toProgress) {
        final ProgressBarAnimation anim = new ProgressBarAnimation(mSeekBar, mFromProgress, toProgress);
        anim.setDuration(200);
        startAnimation(anim);
    }

    private void invokeItemSelected(final int selectedSection) {
        if (mOnItemSelectionListener != null) {
            mOnItemSelectionListener.onItemSelected(selectedSection, getItemString(selectedSection));
        }
    }

    private String getItemString(final int index) {
        if (mItems.length > index) {
            return mItems[index];
        }
        return "";
    }

    @Override
    public void onStartTrackingTouch(final SeekBar seekBar) {
        mFromProgress = mSeekBar.getProgress();
        mThumbPosition = NOT_INITIALIZED_THUMB_POSITION;
    }

    @Override
    public void onStopTrackingTouch(final SeekBar seekBar) {
        handleSnapToClosestValue();
    }

    public Drawable getProgressDrawable() {
        return mProgressDrawable;
    }

    public Drawable getThumb() {
        return mThumbDrawable;
    }

    public int getProgress() {
        return mSeekBar.getProgress();
    }

    public void setProgress(final int progress) {
        mToProgress = progress;
        handleSnapToClosestValue();
    }

    public void setProgressToIndex(final int index) {
        mToProgress = getProgressForIndex(index);
        mSeekBar.setProgress(mToProgress);
    }

    private int getProgressForIndex(final int index) {
        final float sectionLength = 100 / (mItemsAmount - 1);
        return (int) (index * sectionLength);
    }

    public void setProgressToIndexWithAnimation(final int index) {
        mToProgress = getProgressForIndex(index);
        animateProgressBar(mToProgress);
    }

    public int getSelectedItemIndex() {
        final float sectionLength = 100 / (mItemsAmount - 1);
        return (int) ((mToProgress / sectionLength) + 0.5);
    }

    public void setOnItemSelectionListener(final OnItemSelectionListener listener) {
        mOnItemSelectionListener = listener;
    }

    public void setProgressDrawable(final int progressDrawableId) {
        mProgressDrawableId = progressDrawableId;
    }

    public void setThumbDrawable(final int thumbDrawableId) {
        mThumbDrawableId = thumbDrawableId;
    }

    public void setIndicatorDrawable(final int indicatorDrawableId) {
        mIndicatorDrawableId = indicatorDrawableId;
    }

    public void setProgressColor(final int progressColor) {
        mProgressColor = progressColor;
    }

    public void setIndicatorColor(final int indicatorColor) {
        mIndicatorColor = indicatorColor;
    }

    public void setThumbnailColor(final int thumbnailColor) {
        mThumbnailColor = thumbnailColor;
    }

    public void setTextIndicatorColor(final int textIndicatorColor) {
        mTextIndicatorColor = textIndicatorColor;
    }

    public void setTextIndicatorTopMargin(final float textIndicatorTopMargin) {
        mTextIndicatorTopMargin = textIndicatorTopMargin;
    }

    public void setTextStyleId(final int textStyleId) {
        mTextStyleId = textStyleId;
    }

    public void setTextSize(final int textSize) {
        mTextSize = mDensity * textSize;
    }

    public void setIndicatorSize(final int indicatorSize) {
        mIndicatorSize = mDensity * indicatorSize;
    }

    public interface OnItemSelectionListener {
        public void onItemSelected(final int itemIndex, final String itemString);
    }

    static void setColor(final Drawable drawable, final int color) {
        drawable.setColorFilter(new PorterDuffColorFilter(color, PorterDuff.Mode.MULTIPLY));
    }

    static void setLeftMargin(final View view, final int leftMargin) {
        final RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) view.getLayoutParams();
        params.leftMargin = leftMargin;
        view.setLayoutParams(params);
    }

    static void waitForLayoutPrepared(final View view, final LayoutPreparedListener listener) {
        final ViewTreeObserver viewTreeObserver = view.getViewTreeObserver();
        if (viewTreeObserver != null) {
            viewTreeObserver.addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                @Override
                public void onGlobalLayout() {
                    invokeLayoutListener();
                    removeGlobalOnLayoutListenerIfNeeded();
                }

                private void invokeLayoutListener() {
                    if (listener != null) {
                        listener.onLayoutPrepared(view);
                    }
                }

                private void removeGlobalOnLayoutListenerIfNeeded() {
                    final ViewTreeObserver laterViewTreeObserver = view.getViewTreeObserver();
                    if (laterViewTreeObserver != null && laterViewTreeObserver.isAlive()) {
                        laterViewTreeObserver.removeOnGlobalLayoutListener(this);
                    }
                }
            });
        }
    }

    interface LayoutPreparedListener {
        void onLayoutPrepared(final View preparedView);
    }
}
