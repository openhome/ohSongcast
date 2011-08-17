//-----------------------------------------------------------------------------------------
// Based on code published at CodeProject by Murray Foxcroft - April 2009
//-----------------------------------------------------------------------------------------
using System;
using System.Windows.Forms;
using System.Drawing;

namespace OpenHome.Soundcard
{
    /// <summary>
    /// An extension of the notifyIcon Windows Forms class, unfortunately its a 
    //  sealed class so it cannot be inherited. This class adds a timer and additional 
    //  methods and events to allow for monitoring when a mouse enters and leaves the icon area. 
    /// </summary>
    /// 
    public class ExtendedNotifyIcon : IDisposable
    {
        const int kMouseLeaveTimerDelayMilliseconds = 300;

        public EventHandler<EventArgs> Click;
        public EventHandler<EventArgs> RightClick;

        private NotifyIcon iTarget;
        private Timer iTimer;
        private Point iMousePosition;

        private bool iIsMouseOver;

        public ExtendedNotifyIcon(Icon aIcon)
        {
            iIsMouseOver = false;
            iTarget = new NotifyIcon();
            iTarget.Icon = aIcon;
            iTarget.Visible = true;
            iTarget.Click += EventTargetClick;
            iTarget.DoubleClick += EventTargetClick;
            iTarget.MouseMove += EventTargetMouseMove;
            iTarget.MouseClick += EventTargetMouseClick;
            iTimer = new Timer();
            iTimer.Tick += EventTimerTick;
            iTimer.Interval = 100;
        }

        void EventTargetMouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                if (Click != null)
                {
                    Click(this, EventArgs.Empty);
                }
            }
            else if (e.Button == MouseButtons.Right)
            {
                if (RightClick != null)
                {
                    RightClick(this, EventArgs.Empty);
                }
            }
        }

        void EventTimerTick(object sender, EventArgs e)
        {
         // If the mouse position over the icon does not match the screen position, the mouse has left the icon (think of this as a type of hit test) 
            if (iMousePosition != System.Windows.Forms.Control.MousePosition)
            {
                iIsMouseOver = false;
                iTimer.Stop(); // Stop the timer, no longer reqired.
            }
        }

        void EventTargetMouseMove(object sender, MouseEventArgs e)
        {
            iIsMouseOver = true;
            iMousePosition = System.Windows.Forms.Control.MousePosition; // Track the position of the mouse over the notify icon
            iTimer.Start();  // The timer counts down and closes the window, as the mouse moves over the icon, keep starting (resetting) this to stop it from closing the popup
        }

        public ContextMenu ContextMenu
        {
            set
            {
                iTarget.ContextMenu = value;
            }
        }

        public void EventTargetClick(object sender, EventArgs e)
        {
        }

        public bool IsMouseOver
        {
            get
            {
                return (iIsMouseOver);
            }
        }

        public void Dispose()
        {
            iTarget.Dispose();
        }
    }
}
