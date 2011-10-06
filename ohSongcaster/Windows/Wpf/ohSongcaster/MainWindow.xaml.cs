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

namespace OpenHome.Songcaster
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

        private Songcaster iSongcaster;

        private ConfigurationWindow iConfigurationWindow;
        private MediaPlayerWindow iMediaPlayerWindow;

        public MainWindow()
        {
            InitializeComponent();

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

            try
            {
                iSongcaster = new Songcaster("av.openhome.org", iConfigurationWindow.Subnet, iConfigurationWindow.Channel, iConfigurationWindow.Ttl, iConfigurationWindow.Latency, iConfigurationWindow.Multicast, iConfigurationWindow.Enabled, iConfigurationWindow.Preset, iMediaPlayerWindow.ReceiverList, iConfigurationWindow.SubnetList, this, "OpenHome", "http://www.openhome.org", "http://www.openhome.org");
            }
            catch (SongcasterError e)
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

        public bool Enabled
        {
            get
            {
                return (iConfigurationWindow.Enabled);
            }
            set
            {
                iSongcaster.SetEnabled(value);
                iMediaPlayerWindow.SetEnabled(value);
            }
        }

        public void ConfigurationChanged(IConfiguration aConfiguration)
        {
            Dispatcher.BeginInvoke(new Action(SafeConfigurationChanged));
        }

        public void SafeConfigurationChanged()
        {
            bool enabledChanged = false;

            if (iConfigurationWindow.Enabled != iSongcaster.Enabled())
            {
                enabledChanged = true;
            }

            iConfigurationWindow.ConfigurationChanged(iSongcaster);

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
            iSongcaster.RefreshReceivers();
        }

        private void EventSubnetChanged()
        {
            iMediaPlayerWindow.SubnetChanged();
            iSongcaster.SetSubnet(iConfigurationWindow.Subnet);
        }

        private void EventMulticastChanged()
        {
            iSongcaster.SetMulticast(iConfigurationWindow.Multicast);
        }

        private void EventMulticastChannelChanged()
        {
            iSongcaster.SetChannel(iConfigurationWindow.Channel);
        }

        private void EventTtlChanged()
        {
            iSongcaster.SetTtl(iConfigurationWindow.Ttl);
        }

        private void EventLatencyChanged()
        {
            iSongcaster.SetLatency(iConfigurationWindow.Latency);
        }

        private void EventPresetChanged()
        {
            iSongcaster.SetPreset(iConfigurationWindow.Preset);
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
            
            if (iSongcaster != null)
            {
                iSongcaster.Dispose();
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
