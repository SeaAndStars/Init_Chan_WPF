﻿using Init_Chan_WPF.ViewModels.Pages;
using Wpf.Ui.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Init_Chan_WPF.Services;

namespace Init_Chan_WPF.Views.Pages
{
    /// <summary>
    /// Home.xaml 的交互逻辑
    /// </summary>
    public partial class Home : Page
    {
        public Home()
        {
            InitializeComponent();
        }


        private void ToggleSwitch_Checked_1(object sender, RoutedEventArgs e)
        {

        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Main_process.stdprocess("lwBarClientApp32.exe") ;
        }
    }
}
