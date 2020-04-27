using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
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
            MockingEngine.EnsureLoaded();
            MockingEngine.Decorate(typeof(Helper), "GetText", 0, new MockingFunc<string>(OverrideGetText));
        }

        private string OverrideGetText(ref bool handled)
        {
            handled = true;
            return "something special";
        }

        private void button1_Click(object sender, EventArgs e)
        {
            MessageBox.Show(Helper.GetText());
        }
    }

    internal static class Helper
    {
        public static string GetText() => "everything as usual";
    }
}
