package com.lion.rmtsndcli;

import android.os.Handler;

public class UITimer {
    private Handler handler;
    private Runnable runMethod;
    private int intervalMs;
    private boolean enabled = false;
    private boolean oneTime = false;

    public UITimer(Handler handler, Runnable runMethod, int intervalMs)
    {
        this.handler = handler;
        this.runMethod = runMethod;
        this.intervalMs = intervalMs;
    }

    public UITimer(Handler handler, Runnable runMethod, int intervalMs, boolean oneTime)
    {
        this(handler, runMethod, intervalMs);
        this.oneTime = oneTime;
    }

    public void start()
    {
        if (enabled)
            return;

        if (intervalMs < 1)
        {
            //Log.e("timer start", "Invalid interval:" + intervalMs);
        	System.out.println("timer start: Invalid interval:" + intervalMs);
            return;
        }

        enabled = true;
        handler.postDelayed(timer_tick, intervalMs);
    }

    public void stop()
    {
        if (!enabled)
            return;

        enabled = false;
        handler.removeCallbacks(runMethod);
        handler.removeCallbacks(timer_tick);
    }

    public boolean isEnabled()
    {
        return enabled;
    }

    private Runnable timer_tick = new Runnable()
    {
        public void run()
        {
            if (!enabled)
                return;

            handler.post(runMethod);

            if (oneTime)
            {
                enabled = false;
                return;
            }

            handler.postDelayed(timer_tick, intervalMs);
        }
    };
}
