using System;
using System.Collections.Generic;
using System.Configuration;
using System.Windows;

namespace OpenHome.Songcast
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        protected override void OnDeactivated(EventArgs e)
        {
            base.OnDeactivated(e);

            MainWindow main = MainWindow as MainWindow;

            main.ApplicationDeactivated();
        }        
    }
}
