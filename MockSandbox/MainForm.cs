using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace InjectionMocking
{
    internal partial class MainForm : Form
    {
        [STAThread]
        private static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }

        private MainForm()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            var callback1 = GetType().GetMethod(nameof(OverrideGetText), BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance) ?? throw new InvalidOperationException();
            var callback2 = GetType().GetMethod(nameof(OverrideShowValue), BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance) ?? throw new InvalidOperationException();

            MockingEngine.EnsureLoaded();
            MockingEngine.Decorate(typeof(Helper), nameof(Helper.GetText), 0, callback1, null);
            MockingEngine.Decorate(typeof(Helper), nameof(Helper.ShowValue), 1, callback2, null);
        }

        private static string OverrideGetText(ref bool handled)
        {
            handled = true;
            return "something special";
        }

        private static void OverrideShowValue(ref bool handled, int value)
        {
            handled = true;
            MessageBox.Show($"something special: {value}");
        }

        private void button1_Click(object sender, EventArgs e)
        {
            MessageBox.Show(Helper.GetText());
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Helper.ShowValue(42);
        }
    }

    internal static class Helper
    {
        public static string GetText() => "everything as usual";

        public static void ShowValue(int value) => MessageBox.Show($"everything as usual: {value}");
    }
}
