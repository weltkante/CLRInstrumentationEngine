using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using InjectionMocking;

namespace MockSandboxCore
{
    public partial class Form1 : Form
    {
        [STAThread]
        static void Main()
        {
            Application.SetHighDpiMode(HighDpiMode.SystemAware);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            var callback1 = GetType().GetMethod(nameof(OverrideGetText), BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance) ?? throw new InvalidOperationException();
            var callback2 = GetType().GetMethod(nameof(OverrideShowValue), BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance) ?? throw new InvalidOperationException();

            MockingEngine.EnsureLoaded();
            MockingEngine.Decorate(typeof(Helper), nameof(Helper.GetText), 0, callback1, this);
            MockingEngine.Decorate(typeof(Helper), nameof(Helper.ShowValue), 1, callback2, this);
        }

        private string OverrideGetText(ref bool handled)
        {
            handled = true;
            return "something special";
        }

        private void OverrideShowValue(ref bool handled, IntPtr value)
        {
            handled = true;
            MessageBox.Show($"something special: {value.ToInt64()}");
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton2.Checked)
            {
                radioButton1.Checked = true;
                MessageBox.Show(Helper.GetText());
            }
        }

        private unsafe void radioButton3_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton3.Checked)
            {
                radioButton1.Checked = true;
                Helper.ShowValue(new IntPtr(42).ToPointer());
            }
        }
    }

    internal unsafe static class Helper
    {
        public static string GetText() => "everything as usual";

        public static void ShowValue(void* ptrvalue) => MessageBox.Show($"everything as usual: {new IntPtr(ptrvalue).ToInt64()}");
    }
}
