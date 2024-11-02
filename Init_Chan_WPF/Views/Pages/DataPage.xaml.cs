﻿using Init_Chan_WPF.ViewModels.Pages;
using Wpf.Ui.Controls;

namespace Init_Chan_WPF.Views.Pages
{
    public partial class DataPage : INavigableView<DataViewModel>
    {
        public DataViewModel ViewModel { get; }

        public DataPage(DataViewModel viewModel)
        {
            ViewModel = viewModel;
            DataContext = this;

            InitializeComponent();
        }
    }
}
