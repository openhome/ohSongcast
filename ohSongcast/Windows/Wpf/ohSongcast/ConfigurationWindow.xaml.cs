using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.ComponentModel;

namespace OpenHome.Songcast
{
    public class NegateBoolean : IValueConverter
    {
        public object Convert(object aValue, Type aTargetType, object aParameter, System.Globalization.CultureInfo aCulture)
        {
            if (aTargetType != typeof(Nullable<bool>))
            {
                throw new InvalidOperationException("The target must be a boolean");
            }

            return (!(bool)aValue);
        }

        public object ConvertBack(object aValue, Type aTargetType, object aParameter, System.Globalization.CultureInfo aCulture)
        {
            if (aTargetType != typeof(bool))
            {
                throw new InvalidOperationException("The target must be a boolean");
            }

            return (!(bool)aValue);
        }
    }

    /// <summary>
    /// Interaction logic for SettingsWindow.xaml
    /// </summary>
    public partial class ConfigurationWindow : Window, IConfigurationChangedHandler, INotifyPropertyChanged
    {
        private Configuration iConfiguration;
        private SubnetList iSubnetList;

        public event PropertyChangedEventHandler PropertyChanged;

        public ConfigurationWindow()
        {
            InitializeComponent();

            iConfiguration = Configuration.Load();

            iSubnetList = new SubnetList(this.Dispatcher);

            iSubnetList.CountChanged += EventSubnetListCountChanged;

            comboBoxNetwork.ItemsSource = iSubnetList;

            this.DataContext = this;

            buttonChannelNew.Click += EventButtonChannelNewClick;
            comboBoxNetwork.SelectionChanged += EventComboBoxNetworkSelectionChanged;
        }

        public void ConfigurationChanged(IConfiguration aConfiguration)
        {
            Subnet = aConfiguration.Subnet();
            Channel = aConfiguration.Channel();
            Ttl = aConfiguration.Ttl();
            Latency = aConfiguration.Latency();
            Preset = aConfiguration.Preset();

            bool enabled = aConfiguration.Enabled();

            if (iConfiguration.Enabled != enabled)
            {
                iConfiguration.Enabled = enabled;
                iConfiguration.Save();
            }
        }

        void EventSubnetListCountChanged(object sender, EventArgs e)
        {
            if (iConfiguration.Subnet == 0)
            {
                if (iSubnetList.Count > 0)
                {
                    comboBoxNetwork.SelectedIndex = 0;
                    return;
                }
            }

            int index = 0;

            foreach (Subnet subnet in iSubnetList)
            {
                if (subnet.Address == iConfiguration.Subnet)
                {
                    comboBoxNetwork.SelectedIndex = index;
                    return;
                }

                index++;
            }

            comboBoxNetwork.SelectedIndex = -1;
        }

        public SubnetList SubnetList
        {
            get
            {
                return (iSubnetList);
            }
        }

        private void InformListeners(Action aAction)
        {
            if (aAction != null)
            {
                aAction();
            }
        }

        private void EventButtonChannelNewClick(object sender, RoutedEventArgs e)
        {
            Channel = (uint)(new Random().Next(65535) + 1);
        }

        private void EventComboBoxNetworkSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            int index = comboBoxNetwork.SelectedIndex;

            if (index >= 0)
            {
                uint address = iSubnetList.SubnetAt(index).Address;

                if (iConfiguration.Subnet != address)
                {
                    iConfiguration.Subnet = address;
                    iConfiguration.Save();
                    InformListeners(SubnetChanged);
                }
            }
        }

        public Action SubnetChanged;
        public Action MulticastChanged;
        public Action ChannelChanged;
        public Action TtlChanged;
        public Action LatencyChanged;
        public Action PresetChanged;

        public string Version
        {
            get
            {
                return ("Version " + System.Windows.Forms.Application.ProductVersion);
            }
        }

        public uint Subnet
        {
            get
            {
                return (iConfiguration.Subnet);
            }
            set
            {
                if (iConfiguration.Subnet != value)
                {
                    iConfiguration.Subnet = value;
                    iConfiguration.Save();
                    InformListeners(SubnetChanged);

                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("Subnet"));
                    }
                }
            }
        }

        public bool Multicast
        {
            get
            {
                return (iConfiguration.Multicast);
            }
            set
            {
                if (iConfiguration.Multicast != value)
                {
                    iConfiguration.Multicast = value;
                    iConfiguration.Save();
                    InformListeners(MulticastChanged);

                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("Multicast"));
                    }
                }
            }
        }

        public uint Channel
        {
            get
            {
                return (iConfiguration.Channel);
            }
            set
            {
                if (iConfiguration.Channel != value)
                {
                    iConfiguration.Channel = value;
                    iConfiguration.Save();
                    InformListeners(ChannelChanged);

                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("Channel"));
                    }
                }
            }
        }

        public uint Ttl
        {
            get
            {
                return (iConfiguration.Ttl);
            }
            set
            {
                if (iConfiguration.Ttl != value)
                {
                    iConfiguration.Ttl = value;
                    iConfiguration.Save();
                    InformListeners(TtlChanged);

                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("Ttl"));
                    }
                }
            }
        }

        public uint Latency
        {
            get
            {
                return (iConfiguration.Latency);
            }
            set
            {
                if (iConfiguration.Latency != value)
                {
                    iConfiguration.Latency = value;
                    iConfiguration.Save();
                    InformListeners(LatencyChanged);

                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("Latency"));
                    }
                }
            }
        }

        public uint Preset
        {
            get
            {
                return (iConfiguration.Preset);
            }
            set
            {
                if (iConfiguration.Preset != value)
                {
                    iConfiguration.Preset = value;
                    iConfiguration.Save();
                    InformListeners(PresetChanged);

                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("Preset"));
                    }
                }
            }
        }

        public bool Enabled
        {
            get
            {
                return (iConfiguration.Enabled);
            }
        }
    }
}
