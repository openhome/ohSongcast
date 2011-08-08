//-----------------------------------------------------------------------------------------
// Based on code published at CodeProject by Murray Foxcroft - April 2009
//-----------------------------------------------------------------------------------------
using System;
using System.Windows.Forms;
using System.Drawing;

/*
 * 
 * ﻿//-----------------------------------------------------------------------------------------
  2  // Author:   Murray Foxcroft - April 2009
  3  //-----------------------------------------------------------------------------------------
  4  using System;
  5  using System.Windows.Forms;
  6  
  7  namespace ExtendedWindowsControls
  8  {
  9      /// <summary>
 10      /// An extension of the notifyIcon Windows Forms class, unfortunately its a 
 11      //  sealed class so it cannot be inherited. This class adds a timer and additional 
 12      //  methods and events to allow for monitoring when a mouse enters and leaves the icon area. 
 13      /// </summary>
 14      /// 
 15      public class ExtendedNotifyIcon : IDisposable
 16      {
 17          public NotifyIcon targetNotifyIcon;
 18          private System.Drawing.Point notifyIconMousePosition;
 19          private Timer delayMouseLeaveEventTimer;
 20      
 21          public delegate void MouseLeaveHandler();
 22          public event MouseLeaveHandler MouseLeave;
 23  
 24          public delegate void MouseMoveHandler();
 25          public event MouseMoveHandler MouseMove;
 26  
 27          /// <summary>
 28          /// Constructor
 29          /// </summary>
 30          /// <param name="millisecondsToDelayMouseLeaveEvent"></param>
 31          public ExtendedNotifyIcon(int millisecondsToDelayMouseLeaveEvent)
 32          {
 33              // Configure and show a notification icon in the system tray
 34              targetNotifyIcon = new NotifyIcon();
 35              targetNotifyIcon.Visible = true;
 36              targetNotifyIcon.MouseMove += new MouseEventHandler(targetNotifyIcon_MouseMove);
 37  
 38              delayMouseLeaveEventTimer = new Timer();
 39              delayMouseLeaveEventTimer.Tick += new EventHandler(delayMouseLeaveEventTimer_Tick);
 40              delayMouseLeaveEventTimer.Interval = 100;
 41          }
 42  
 43          /// <summary>
 44          /// Chained constructor - default millisecondsToDelayMouseLeaveEvent is 100ms
 45          /// </summary>
 46          public ExtendedNotifyIcon() : this(100) { }
 47  
 48          /// <summary>
 49          /// Manual override exposed - START the timer which will ultimately trigger the mouse leave event
 50          /// </summary>
 51          public void StartMouseLeaveTimer()
 52          {
 53              delayMouseLeaveEventTimer.Start();
 54          }
 55  
 56          /// <summary>
 57          /// Manual override exposed - STOP the timer that would ultimately close the window
 58          /// </summary>
 59          public void StopMouseLeaveEventFromFiring()
 60          {
 61              delayMouseLeaveEventTimer.Stop();
 62          }
 63  
 64          /// <summary>
 65          /// If the mouse is moving over the notify icon, the popup must be displayed.     
 66          /// Note: There is no event on the notify icon to trap when the mouse leave, so a timer is used in conjunction 
 67          /// with tracking the position of the mouse to test for when the popup window needs to be closed. See timer tick event.
 68          /// </summary>
 69          /// <param name="sender"></param>
 70          /// <param name="e"></param>
 71          public void targetNotifyIcon_MouseMove(object sender, MouseEventArgs e)
 72          {
 73              notifyIconMousePosition = System.Windows.Forms.Control.MousePosition; // Track the position of the mouse over the notify icon
 74              MouseMove(); // The mouse is moving over the notify Icon, raise the event
 75              delayMouseLeaveEventTimer.Start();  // The timer counts down and closes the window, as the mouse moves over the icon, keep starting (resetting) this to stop it from closing the popup
 76          }
 77  
 78          /// <summary>
 79          /// Under the right conditions, raise the event to the popup window to tell it to close.
 80          /// </summary>
 81          /// <param name="sender"></param>
 82          /// <param name="e"></param>
 83          void delayMouseLeaveEventTimer_Tick(object sender, EventArgs e)
 84          {
 85              // If the mouse position over the icon does not match the sryceen position, the mouse has left the icon (think of this as a type of hit test) 
 86              if (notifyIconMousePosition != System.Windows.Forms.Control.MousePosition)
 87              {
 88                  MouseLeave();  // Raise the event for the mouse leaving 
 89                  delayMouseLeaveEventTimer.Stop(); // Stop the timer, no longer reqired.
 90              }
 91          }
 92  
 93          #region IDisposable Members
 94  
 95          /// <summary>
 96          /// Standard IDisposable interface implementation. If you dont dispose the windows notify icon, the application
 97          /// closes but the icon remains in the task bar until such time as you mouse over it.
 98          /// </summary>
 99          private bool _IsDisposed = false;
100  
101          ~ExtendedNotifyIcon()
102          {
103              Dispose(false);
104          }
105  
106          public void Dispose()
107          {
108              Dispose(true);
109              // Tell the garbage collector not to call the finalizer
110              // since all the cleanup will already be done.
111              GC.SuppressFinalize(true);
112          }
113  
114          protected virtual void Dispose(bool IsDisposing)
115          {
116              if (_IsDisposed)
117                  return;
118  
119              if (IsDisposing)
120              {
121                  targetNotifyIcon.Dispose();
122              }
123  
124              // Free any unmanaged resources in this section
125              _IsDisposed = true;
126  
127          #endregion
128          }
129      }
130  }
 */

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
                Console.WriteLine("Click");

                if (Click != null)
                {
                    Click(this, EventArgs.Empty);
                }
            }
            else if (e.Button == MouseButtons.Right)
            {
                Console.WriteLine("Right Click");

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

        private bool iIsMouseOver;

        public void Dispose()
        {
            iTarget.Dispose();
        }
    }
}
