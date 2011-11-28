using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Net;
using System.ComponentModel;
using System.Collections.ObjectModel;
using System.Collections.Specialized;

using System.Xml;
using System.Xml.Serialization;
using System.IO;
using System.Runtime.InteropServices;

namespace OpenHome.Songcast
{

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, IRefreshHandler, IConfigurationChangedHandler, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private ExtendedNotifyIcon iExtendedNotifyIcon; // global class scope for the icon as it needs to exist foer the lifetime of the window
        private Storyboard iStoryBoardFadeIn;
        private Storyboard iStoryBoardFadeOut;

        private Songcast iSongcast;

        private ConfigurationWindow iConfigurationWindow;
        private MediaPlayerWindow iMediaPlayerWindow;

        public MainWindow()
        {
            InitializeComponent();

            AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(EventCurrentDomainUnhandledException);

            this.DataContext = this;

            iExtendedNotifyIcon = new ExtendedNotifyIcon(Properties.Resources.Icon);
            iExtendedNotifyIcon.Click += EventNotifyIconClick;
            iExtendedNotifyIcon.RightClick += EventNotifyIconRightClick;

            System.Windows.Forms.ContextMenu menu = new System.Windows.Forms.ContextMenu();

            System.Windows.Forms.MenuItem open = new System.Windows.Forms.MenuItem("Open", new EventHandler(EventContextMenuOpen));
            System.Windows.Forms.MenuItem exit = new System.Windows.Forms.MenuItem("Exit", new EventHandler(EventContextMenuExit));

            menu.MenuItems.Add(open);
            menu.MenuItems.Add(exit);

            iExtendedNotifyIcon.ContextMenu = menu;

            Left = SystemParameters.WorkArea.Width - LayoutRoot.Width - 10;
            Top = SystemParameters.WorkArea.Height - LayoutRoot.Height - 2;

            iConfigurationWindow = new ConfigurationWindow();
            iMediaPlayerWindow = new MediaPlayerWindow(iConfigurationWindow.Enabled, this);

            iConfigurationWindow.Left = Left;
            iConfigurationWindow.Top = Top - iConfigurationWindow.LayoutRoot.Height + 1;

            iMediaPlayerWindow.Left = Left;
            iMediaPlayerWindow.Top = Top - iMediaPlayerWindow.LayoutRoot.Height + 1;

            // Locate these storyboards and "cache" them - we only ever want to find these once for performance reasons
            iStoryBoardFadeIn = (Storyboard)this.TryFindResource("storyBoardFadeIn");
            iStoryBoardFadeIn.Completed += new EventHandler(EventStoryBoardFadeInCompleted);
            iStoryBoardFadeOut = (Storyboard)TryFindResource("storyBoardFadeOut");
            iStoryBoardFadeOut.Completed += new EventHandler(EventStoryBoardFadeOutCompleted);

            this.Closing += EventWindowClosing;

            System.Drawing.Image image = OpenHome.Songcast.Properties.Resources.Icon.ToBitmap();

            MemoryStream stream = new MemoryStream();

            image.Save(stream, System.Drawing.Imaging.ImageFormat.Bmp);

            byte[] bytes = stream.ToArray();

            try
            {
                iSongcast = new Songcast("av.openhome.org", iConfigurationWindow.Subnet, iConfigurationWindow.Channel, iConfigurationWindow.Ttl, iConfigurationWindow.Latency, iConfigurationWindow.Multicast, iConfigurationWindow.Enabled, iConfigurationWindow.Preset, iMediaPlayerWindow.ReceiverList, iConfigurationWindow.SubnetList, this, "OpenHome", "http://www.openhome.org", "http://www.openhome.org", bytes, "image/bmp");
            }
            catch (SongcastError e)
            {
                MessageBox.Show(e.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                App.Current.Shutdown(1);
            }

            iConfigurationWindow.SubnetChanged += EventSubnetChanged;
            iConfigurationWindow.MulticastChanged += EventMulticastChanged;
            iConfigurationWindow.ChannelChanged += EventMulticastChannelChanged;
            iConfigurationWindow.TtlChanged += EventTtlChanged;
            iConfigurationWindow.LatencyChanged += EventLatencyChanged;
            iConfigurationWindow.PresetChanged += EventPresetChanged;

            bool value = iConfigurationWindow.Enabled;

            iMediaPlayerWindow.SetEnabled(value);

            Settings.Click += new RoutedEventHandler(EventSettingsClick);
            Receivers.Click += new RoutedEventHandler(EventReceiversClick);

            this.Topmost = true;
            iConfigurationWindow.Topmost = true;
            iMediaPlayerWindow.Topmost = true;
        }

        const Int32 EXCEPTION_MAXIMUM_PARAMETERS = 15; // maximum number of exception parameters


        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
        struct MySEHExceptionStruct
        {
            [MarshalAs(UnmanagedType.LPStr)]
            public string m_strMessage1;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst=256)]
            public string m_strMessage2;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        struct EXCEPTION_POINTERS
        {
            public IntPtr pExceptionRecord; // Points to a EXCEPTION_RECORD record.
            public IntPtr pContext; // Points to a CONTEXT record. This structure contains processor-specific register data.
        }
        
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        struct EXCEPTION_RECORD
        {
            public UInt32 ExceptionCode;
            public UInt32 ExceptionFlags;
            public IntPtr pExceptionRecord; // Points to another EXCEPTION_RECORD structure.
            public IntPtr ExceptionAddress;
            public UInt32 NumberParameters;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = EXCEPTION_MAXIMUM_PARAMETERS)]
            public UInt32[] ExceptionInformation;
        }

        void EventCurrentDomainUnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            try
            {
                throw (e.ExceptionObject as Exception);
            }
            catch (SEHException x)
            
            {
               // Marshal.GetExceptionCode() returns the exception code which was passed
                // by as the first parameter to RaiseException().
                int iExceptionCode = Marshal.GetExceptionCode();
                // Get a pointer to the unmanaged EXCEPTION_POINTERS structure.
                IntPtr pExceptionPointers = Marshal.GetExceptionPointers();

                // Convert the unmanaged EXCEPTION_POINTERS structure into its managed version
                // using Marshal.PtrToStructure().
                EXCEPTION_POINTERS exception_pointers
                  = (EXCEPTION_POINTERS)Marshal.PtrToStructure(pExceptionPointers, typeof(EXCEPTION_POINTERS));
                // Convert the unmanaged EXCEPTION_RECORD structure into its managed version
                // using Marshal.PtrToStructure().
                EXCEPTION_RECORD exception_record
                  = (EXCEPTION_RECORD)Marshal.PtrToStructure
                    (
                      exception_pointers.pExceptionRecord,
                      typeof(EXCEPTION_RECORD)
                    );

                // Check the exception code. If it is one we recognize, we proceed
                // to extract our customized exception information, e.i. a pointer
                // to an MyException structure.
                if (((UInt32)iExceptionCode == 100) && (exception_record.NumberParameters > 0))
                {
                    // The exception_record.ExceptionInformation[] array contains user-defined
                    // data. The first item is a pointer to the unmanaged MySEHExceptionStruct
                    // C++ structure.
                    // We must convert it into a managed version of the MySEHExceptionStruct
                    // structure.
                    MySEHExceptionStruct my_seh_exception_structure
                      = (MySEHExceptionStruct)Marshal.PtrToStructure
                        (
                          (IntPtr)(exception_record.ExceptionInformation[0]),
                          typeof(MySEHExceptionStruct)
                        );
                    // Display info on exception.
                    Console.WriteLine("Exception code : {0:D}.", iExceptionCode);
                    Console.WriteLine("Exception Message 1 : {0:S}", my_seh_exception_structure.m_strMessage1);
                    Console.WriteLine("Exception Message 2 : {0:S}", my_seh_exception_structure.m_strMessage2);
                    // It is the responsibility of the recipient of the exception to free
                    // the memory occupied by the unmanaged MySEHExceptionStruct as well
                    // as members of the unmanaged MySEHExceptionStruct which are references
                    // to other memory. We do this by calling Marshal.DestroyStructure().
                    //
                    // Marshal.DestroyStructure() requires adequate information for the fields
                    // of the target structure to destroy (in the form of MarshalAsAttributes).
                    Marshal.DestroyStructure((IntPtr)(exception_record.ExceptionInformation[0]), typeof(MySEHExceptionStruct));
                    // Finally, free the unmanaged MySEHExceptionStruct structure itself.
                    Marshal.FreeCoTaskMem((IntPtr)(exception_record.ExceptionInformation[0]));
                }
                MessageBox.Show(x.ToString());
            }
            catch (Exception x)
            {
                MessageBox.Show(x.ToString());
            }

        }

        public bool Enabled
        {
            get
            {
                return (iConfigurationWindow.Enabled);
            }
            set
            {
                iSongcast.SetEnabled(value);
                iMediaPlayerWindow.SetEnabled(value);
            }
        }

        public void ConfigurationChanged(IConfiguration aConfiguration)
        {
            Dispatcher.Invoke(new Action(SafeConfigurationChanged));
        }

        public void SafeConfigurationChanged()
        {
            bool enabledChanged = false;

            if (iConfigurationWindow.Enabled != iSongcast.Enabled())
            {
                enabledChanged = true;
            }

            iConfigurationWindow.ConfigurationChanged(iSongcast);

            iMediaPlayerWindow.SetEnabled(iConfigurationWindow.Enabled);

            if (enabledChanged)
            {
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("Enabled"));
                }
            }

        }

        public void Refresh()
        {
            iSongcast.RefreshReceivers();
        }

        private void EventSubnetChanged()
        {
            iMediaPlayerWindow.SubnetChanged();
            iSongcast.SetSubnet(iConfigurationWindow.Subnet);
        }

        private void EventMulticastChanged()
        {
            iSongcast.SetMulticast(iConfigurationWindow.Multicast);
        }

        private void EventMulticastChannelChanged()
        {
            iSongcast.SetChannel(iConfigurationWindow.Channel);
        }

        private void EventTtlChanged()
        {
            iSongcast.SetTtl(iConfigurationWindow.Ttl);
        }

        private void EventLatencyChanged()
        {
            iSongcast.SetLatency(iConfigurationWindow.Latency);
        }

        private void EventPresetChanged()
        {
            iSongcast.SetPreset(iConfigurationWindow.Preset);
        }

        private void EventContextMenuExit(object sender, EventArgs e)
        {
            this.Close();
        }

        private void EventSettingsClick(object sender, RoutedEventArgs e)
        {
            bool enabled = Settings.IsChecked.Value;

            if (enabled)
            {
                iConfigurationWindow.Visibility = Visibility.Visible;
                iMediaPlayerWindow.Visibility = Visibility.Collapsed;
                Receivers.IsChecked = false;
            }
            else
            {
                iConfigurationWindow.Visibility = Visibility.Collapsed;
            }
        }

        private void EventReceiversClick(object sender, RoutedEventArgs e)
        {
            bool enabled = Receivers.IsChecked.Value;

            if (enabled)
            {
                iMediaPlayerWindow.Visibility = Visibility.Visible;
                iConfigurationWindow.Visibility = Visibility.Collapsed;
                Settings.IsChecked = false;
            }
            else
            {
                iMediaPlayerWindow.Visibility = Visibility.Collapsed;
            }
        }

        private void EventWindowClosing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            iConfigurationWindow.Close();
            iMediaPlayerWindow.SetEnabled(false);
            iMediaPlayerWindow.Close();
            iExtendedNotifyIcon.Dispose();
            
            if (iSongcast != null)
            {
                iSongcast.Dispose();
            }
        }

        private void EventStoryBoardFadeInCompleted(object sender, EventArgs e)
        {
        }

        private void EventStoryBoardFadeOutCompleted(object sender, EventArgs e)
        {
            this.Visibility = Visibility.Collapsed;
        }

        void EventNotifyIconClick(object sender, EventArgs e)
        {
            if (Visibility == Visibility.Visible)
            {
                HideOurselves();
            }
            else
            {
                ShowOurselves();
            }
        }

        private void EventContextMenuOpen(object sender, EventArgs e)
        {
            ShowOurselves();
        }

        void HideOurselves()
        {
            iConfigurationWindow.Visibility = Visibility.Collapsed;
            iMediaPlayerWindow.Visibility = Visibility.Collapsed;
            this.Visibility = Visibility.Collapsed;
        }

        void ShowOurselves()
        {
            Visibility = Visibility.Visible;

            Activate();

            if (Settings.IsChecked.Value)
            {
                iConfigurationWindow.Visibility = Visibility.Visible;
                iConfigurationWindow.Activate();
            }

            if (Receivers.IsChecked.Value)
            {
                iMediaPlayerWindow.Visibility = Visibility.Visible;
                iMediaPlayerWindow.Activate();
            }
        }

        void EventNotifyIconRightClick(object sender, EventArgs e)
        {
        }

        public void ApplicationDeactivated()
        {
            if (iExtendedNotifyIcon.IsMouseOver)
            {
                return;
            }

            HideOurselves();
        }
    }
}
