using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Security.Policy;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using System.Threading;
using System.Collections;


namespace ODETRON_AMM_SERIAL_PORT_INTERFACE
{
    public partial class Form1 : Form
    {
        //Crc value
        const ushort polynomial = 0xA001;
        ushort[] table = new ushort[256];

        //Create two different encodings for motion sensor format packet
        Encoding ascii = Encoding.ASCII;
        Encoding unicode = Encoding.Unicode;

        string sendWith;
        string dataIN;
        int dataINLength;
        int[] dataInDec;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            //User Application Initialize Condition
            lblSystemTestStatus.Text = "NO TEST STARTED";
            lblLed1Status.Text = "OFF";
            lblStatusLed2.Text = "OFF";
            lblStatusBuzzer.Text = "OFF";
            lblStatusSpeaker.Text = "OFF";
            lblStatusMotor.Text = "STOP";
            btnLed1Status.BackColor = SystemColors.Control;
            btnLed2Status.BackColor = SystemColors.Control;
            btnBuzzerStatus.BackColor = SystemColors.Control;
            btnSpeakerStatus.BackColor = SystemColors.Control;
            btnLed1Status.Enabled = false;
            btnSystemStatusLed1.BackColor = SystemColors.Control;
            btnSystemStatusLed2.BackColor = SystemColors.Control;
            btnSystemStatusBuzzer.BackColor = SystemColors.Control;
            btnSystemStatusSpeaker.BackColor = SystemColors.Control;
            btnSystemStatusMotorAuto.BackColor = SystemColors.Control;
            btnSystemStatusMotorManuel.BackColor = SystemColors.Control;
            btnSystemStatusMotorStop.BackColor = SystemColors.Control;

            //radio button section
            radioButtLedTest.Checked = false;
            radioButtBuzzerTest.Checked = false;
            radioButtSpeakerTest.Checked = false;
            radioButtMotorTest.Checked = false;

            //Initialize Condition
            chBoxDtrEnable.Checked = false;
            serialPort1.DtrEnable = false;
            chBoxRtsEnable.Checked = false;
            serialPort1.RtsEnable = false;
            btnSendData.Enabled = true;
            sendWith = "Both";

            toolStripComboBox1.Text = "Add to Old Data";
            toolStripComboBox2.Text = "Both";
            toolStripComboBox3.Text = "BUTTON";
            toolStripComboBox4.Text = "Char";
            toolStripComboBox_TxDataFormat.Text = "Char";
            lblStatusCom.Text = "OFF";

            this.toolStripComboBox_TxDataFormat.ComboBox.SelectionChangeCommitted += new System.EventHandler
                (this.toolStripComboBox_TxDataFormat_SelectionChangeCommitted);
        }

        private void btnOpen_Click(object sender, EventArgs e)
        {
            try
            {
                serialPort1.PortName = cBoxCOMPORT.Text;
                serialPort1.BaudRate = Convert.ToInt32(cBoxBAUDRATE.Text);
                serialPort1.DataBits = Convert.ToInt32(cBoxDATABITS.Text);
                serialPort1.StopBits = (StopBits)Enum.Parse(typeof(StopBits), cBoxSTOPBITS.Text);
                serialPort1.Parity = (Parity)Enum.Parse(typeof(Parity), cBoxPARITYBITS.Text);

                serialPort1.Open();
                progressBar1.Value = 100;
                lblStatusCom.Text = "ON";
            }
            catch (Exception err)
            {
                MessageBox.Show(err.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                serialPort1.Close();
                progressBar1.Value = 0;
                lblStatusCom.Text = "OFF";
            }
        }

        private void toolStripComboBox2_DropDownClosed(object sender, EventArgs e)
        {
            if (toolStripComboBox2.Text == "None")
            {
                sendWith = "None";
            }
            else if (toolStripComboBox2.Text == "Both")
            {
                sendWith = "Both";
            }
            else if (toolStripComboBox2.Text == "New Line")
            {
                sendWith = "New Line";
            }
            else if (toolStripComboBox2.Text == "Carriage Return")
            {
                sendWith = "Carriage Return";
            }
        }

        private void btnClearDataOUT_Click(object sender, EventArgs e)
        {
            if (tBoxDataOut.Text != "") //Eğer TextBox doluysa yani içinde metin varsa
            {
                tBoxDataOut.Text = ""; //TexBox içindeki metni sil
            }
        }

        private void tBoxDataOut_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter) //eğer enter tuşuna basılırsa
            {
                this.doSomething();
                e.Handled = true;
                e.SuppressKeyPress = true;  //ding dong sesini önlemek için
            }
        }

        private void doSomething()
        {
            TxSendData();
        }

        private void serialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            //dataIN = serialPort1.ReadExisting();            //This methode show the data serial into TextBox


            List<int> dataBuffer = new List<int>();

            while (serialPort1.BytesToRead > 0)
            {
                try
                {
                    dataBuffer.Add(serialPort1.ReadByte());
                }
                catch (Exception error)
                {
                    MessageBox.Show(error.Message);
                }
            }

            dataINLength = dataBuffer.Count();
            dataInDec = new int[dataINLength];
            dataInDec = dataBuffer.ToArray();

            this.Invoke(new EventHandler(ShowData));        //You cant show the data to the TextBox directly without using this methode
        }

        private string RxDataFormat(int[] dataInput)
        {
            string strOut = "";

            if (toolStripComboBox4.Text == "Hex")
            {
                foreach (int element in dataInput)
                {
                    strOut += Convert.ToString(element, 16) + "\t";
                }
            }
            else if (toolStripComboBox4.Text == "Decimal")
            {
                foreach (int element in dataInput)
                {
                    strOut += Convert.ToString(element) + "\t";
                }
            }
            else if (toolStripComboBox4.Text == "Binary")
            {
                foreach (int element in dataInput)
                {
                    strOut += Convert.ToString(element, 2) + "\t";
                }
            }
            else if (toolStripComboBox4.Text == "Char")
            {
                foreach (int element in dataInput)
                {
                    strOut += Convert.ToChar(element);
                }
            }

            return strOut;
        }

        private void ShowData(object sender, EventArgs e)
        {
            // Convert the string into a byte array.
            //

            // Perform the conversion from one encoding to the other.
            //byte[] asciiBytes = Encoding.Convert(unicode, ascii, unicodeBytes);

            //int dataINLength = tBoxDataIN.Text.Length; //This is for serial data input length
            dataIN = RxDataFormat(dataInDec);
            dataINLength = tBoxDataIN.TextLength;

            tBoxRxLength.Text = string.Format("{0:00}", dataINLength); //Not for textbox dataIN content length

            if (toolStripComboBox1.Text == "Always Update")
            {
                tBoxDataIN.Text = dataIN;
            }
            else if (toolStripComboBox1.Text == "Add to Old Data")
            {
                if (toolStripComboBox3.Text == "TOP")
                {
                    // Gelen veriyi en başa ekle ve ardından mevcut metni ekle
                    tBoxDataIN.Text = dataIN + tBoxDataIN.Text;

                }
                else if (toolStripComboBox3.Text == "BUTTON")
                {
                    // Gelen veriyi kontrol et ve satır sonu karakterlerini değiştir
                    string formattedDataIN = dataIN.Replace("\n", Environment.NewLine);

                    // Metin kutusuna ekle
                    tBoxDataIN.AppendText(formattedDataIN);

                    // İmleci metnin sonuna taşı ve oraya kaydır
                    tBoxDataIN.SelectionStart = tBoxDataIN.Text.Length;
                    tBoxDataIN.SelectionLength = 0; // İmleci göster
                    tBoxDataIN.ScrollToCaret(); //textboxta metin dolduğunda otomatik aşağı kaydırır
                }
            }
        }

        private void btnClearDataIN_Click(object sender, EventArgs e)
        {
            if (tBoxDataIN.Text != "")
            {
                tBoxDataIN.Text = "";
            }
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                serialPort1.PortName = cBoxCOMPORT.Text;
                serialPort1.BaudRate = Convert.ToInt32(cBoxBAUDRATE.Text);
                serialPort1.DataBits = Convert.ToInt32(cBoxDATABITS.Text);
                serialPort1.StopBits = (StopBits)Enum.Parse(typeof(StopBits), cBoxSTOPBITS.Text);
                serialPort1.Parity = (Parity)Enum.Parse(typeof(Parity), cBoxPARITYBITS.Text);

                serialPort1.Open();
                progressBar1.Value = 100;
                lblStatusCom.Text = "ON";
            }
            catch (Exception err)
            {
                MessageBox.Show(err.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void closeEraseOperation()
        {
            btnLed1Status.BackColor = SystemColors.Control;

            byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x04, 0x2C, 0x0B, 0x00, 0x2C, 0x21, 0x97, 0xA0 };

            // Seri port üzerinden veriyi gönder
            serialPort1.Write(dataToSend, 0, dataToSend.Length);

            //Etiket durumu değiştir
            lblLed1Status.Text = "OFF";

            dataToSend = new byte[] { 0x24, 0x2C, 0x05, 0x2C, 0x0C, 0x00, 0x2C, 0x21, 0x32, 0xA0 };

            // Seri port üzerinden veriyi gönder
            serialPort1.Write(dataToSend, 0, dataToSend.Length);

            //Etiket durumu değiştir
            lblStatusLed2.Text = "OFF";

            btnLed2Status.BackColor = SystemColors.Control;

            dataToSend = new byte[] { 0x24, 0x2C, 0x06, 0x2C, 0x0D, 0x00, 0x2C, 0x21, 0xFD, 0xA1 };

            // Seri port üzerinden veriyi gönder
            serialPort1.Write(dataToSend, 0, dataToSend.Length);

            //Etiket durumu değiştir
            lblStatusBuzzer.Text = "OFF";

            btnBuzzerStatus.BackColor = SystemColors.Control;

            dataToSend = new byte[] { 0x24, 0x2C, 0x01, 0x2C, 0x0A, 0x00, 0x2C, 0x21, 0x3E, 0xA1 };

            // Seri port üzerinden veriyi gönder
            serialPort1.Write(dataToSend, 0, dataToSend.Length);

            //Etiket durumu değiştir
            lblStatusSpeaker.Text = "OFF";

            btnSpeakerStatus.BackColor = SystemColors.Control;

            dataToSend = new byte[] { 0x24, 0x2C, 0x07, 0x2C, 0x0D, 0x01, 0x2C, 0x21, 0xEC, 0xF1 };

            // Seri port üzerinden veriyi gönder
            serialPort1.Write(dataToSend, 0, dataToSend.Length);

            //Etiket durumu değiştir
            lblStatusMotor.Text = "STOP";


            btnSystemStatusLed1.BackColor = SystemColors.Control;
            btnSystemStatusLed2.BackColor = SystemColors.Control;
            btnSystemStatusBuzzer.BackColor = SystemColors.Control;
            btnSystemStatusSpeaker.BackColor = SystemColors.Control;
            btnSystemStatusMotorManuel.BackColor = SystemColors.Control;
            btnSystemStatusMotorAuto.BackColor = SystemColors.Control;
            btnSystemStatusMotorStop.BackColor = SystemColors.Control;
        }

        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                closeEraseOperation();
                serialPort1.Close();
                progressBar1.Value = 0;
                lblStatusCom.Text = "OFF";
            }
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Created by Kubilay Közleme", "Creator", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void btnSendData_Click(object sender, EventArgs e)
        {
            TxSendData();
        }

        private void cBoxCOMPORT_DropDown(object sender, EventArgs e)
        {
            string[] ports = SerialPort.GetPortNames();
            cBoxCOMPORT.Items.Clear();
            cBoxCOMPORT.Items.AddRange(ports);      //aktif portlar arasında geçiş yapmayı sağlar
        }

        private void TxDataFormat()
        {
            if (toolStripComboBox_TxDataFormat.Text == "Char")
            {
                //Send the data in the textbox via serial port
                serialPort1.Write(tBoxDataOut.Text);

                //Calculate the length of the data send and then show it
                int dataOUTLength = tBoxDataOut.Text.Length;
                tBoxTxLength.Text = string.Format("{0:00}", dataOUTLength); //bu kod iki basamaklı string formatını tutmak içindir
            }
            else
            {
                //Declare Local Variable
                string dataOutBuffer;
                int countComma = 0;
                string[] dataPrepareToSend;
                byte[] dataToSend;

                try
                {
                    //Move the data package in the textbox into a variable
                    dataOutBuffer = tBoxDataOut.Text;

                    //Count how much comma (,) punctuation in the data package
                    foreach (char c in dataOutBuffer) { if (c == ',') { countComma++; } }

                    //Create one-dimensional array (string data type) with the length based on the countComma
                    dataPrepareToSend = new string[countComma];

                    //Parsing the data in dataOutBuffer and save it into an array dataPrepareToSend based on comma punctuation
                    countComma = 0; //Reset Value to 0
                    foreach (char c in dataOutBuffer)
                    {
                        if (c != ',')
                        {
                            //Append the data to array of dataPrepareToSend
                            dataPrepareToSend[countComma] += c;
                        }
                        else
                        {
                            //If a comma finds in the data package, then increase the countComma variable.
                            //CountComma is using to determine the index of dataPrepareToSend array
                            countComma++;
                            //Stop foreach process if numbers of countComma equal to the size of dataPrepareToSend
                            if (countComma == dataPrepareToSend.GetLength(0)) { break; };
                        }
                    }

                    //Create one-dimensional array (byte data type) with the length based on the size of dataPrepareToSend
                    dataToSend = new byte[dataPrepareToSend.Length];

                    if (toolStripComboBox_TxDataFormat.Text == "Hex")
                    {
                        //Convert data in string array (dataPrepareToSend) into byte array(dataToSend)
                        for (int a = 0; a < dataPrepareToSend.Length; a++)
                        {
                            dataToSend[a] = Convert.ToByte(dataPrepareToSend[a], 16);
                            //Convert string to an 8-bit unsigned integer with the specified base number
                            //Value 16 mean Hex
                        }
                    }
                    else if (toolStripComboBox_TxDataFormat.Text == "Binary")
                    {
                        //Convert data in string array (dataPrepareToSend) into byte array(dataToSend)
                        for (int a = 0; a < dataPrepareToSend.Length; a++)
                        {
                            dataToSend[a] = Convert.ToByte(dataPrepareToSend[a], 2);
                            //Convert string to an 8-bit unsigned integer with the specified base number
                            //Value 2 mean Binary
                        }
                    }
                    else if (toolStripComboBox_TxDataFormat.Text == "Decimal")
                    {
                        //Convert data in string array (dataPrepareToSend) into byte array(dataToSend)
                        for (int a = 0; a < dataPrepareToSend.Length; a++)
                        {
                            dataToSend[a] = Convert.ToByte(dataPrepareToSend[a], 10);
                            //Convert string to an 8-bit unsigned integer with the specified base number
                            //Value 10 mean Decimal
                        }
                    }

                    //Send a spicified number of bytes to the serial port
                    serialPort1.Write(dataToSend, 0, dataToSend.Length);

                    //Calculate the length of data sent and then show it
                    tBoxTxLength.Text = string.Format("{0:00}", dataToSend.Length); //bu kod iki basamaklı string formatını tutmak içindir
                }
                catch (Exception error)
                {
                    MessageBox.Show(error.Message);
                }
            }
        }

        private void TxSendData()
        {
            if (serialPort1.IsOpen)
            {
                //dataOUT = tBoxDataOut.Text;
                if (sendWith == "None")
                {
                    //serialPort1.Write(dataOUT);
                    TxDataFormat();
                }
                else if (sendWith == "Both")
                {
                    //serialPort1.Write(dataOUT + "\r\n");
                    TxDataFormat();
                    serialPort1.Write("\r\n");
                }
                else if (sendWith == "New Line")
                {
                    //serialPort1.Write(dataOUT + "\n");
                    TxDataFormat();
                    serialPort1.Write("\n");
                }
                else if (sendWith == "Carriage Return")
                {
                    //serialPort1.Write(dataOUT + "\r");
                    TxDataFormat();
                    serialPort1.Write("\r");
                }
            }



            if (clearToolStripMenuItem.Checked)
            {
                if (tBoxDataOut.Text != "") //Eğer TextBox doluysa yani içinde metin varsa
                {
                    tBoxDataOut.Text = ""; //TexBox içindeki metni sil
                }
            }
        }

        private void clearToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            if (tBoxDataIN.Text != "")
            {
                tBoxDataIN.Text = "";
            }
        }

        private void tBoxDataOut_KeyPress(object sender, KeyPressEventArgs e)
        {
            char c = e.KeyChar;

            if (toolStripComboBox_TxDataFormat.Text == "Hex")
            {
                //In Hex format  , the textbox only accepts the 0-9 key and A-F key
                //The lower case will convert to upper case , so both can enter on the textbox
                char uppercase = char.ToUpper(c);

                //if it is not the numbers key pressed , not the backspace key pressed , not the delete key
                //pressed, not a comma key pressed , not the A-F key pressed
                if (!char.IsDigit(uppercase) && uppercase != 8 && uppercase != 46 && uppercase != ',' && !
                    (uppercase >= 65 && uppercase <= 70))
                {
                    //Cancel the KeyPress Event
                    e.Handled = true;
                }

            }
            else if (toolStripComboBox_TxDataFormat.Text == "Decimal")
            {
                //In Decimal format  , the textbox only accepts the numbers key, that is 0-9

                //if it is not the numbers key pressed , not the backspace key pressed , not the delete key
                //pressed, not a comma key pressed
                if (!char.IsDigit(c) && c != 8 && c != 46 && c != ',')
                {
                    //Cancel the KeyPress Event
                    e.Handled = true;
                }
            }
            else if (toolStripComboBox_TxDataFormat.Text == "Binary")
            {
                //In Binary format  , the textbox only take 1 and 0 key

                //if it is not the one (1) key pressed , not the zero (0) key pressed , not the backspace key pressed , not the delete key
                //pressed, not a comma key pressed
                if (c != 49 && c != 48 && c != 8 && c != 46 && c != ',')
                {
                    //Cancel the KeyPress Event
                    e.Handled = true;
                }
            }
            else if (toolStripComboBox_TxDataFormat.Text == "Char")
            {
                //Do nothing
            }
        }

        private void toolStripComboBox_TxDataFormat_SelectionChangeCommitted(object sender, EventArgs e)
        {
            tBoxDataOut.Clear();

            string message = "If you are not using char data format, append the comma (,) after each byte data. Otherwise, the byte data will ignore. \n" +
                "Example : \n\t255,  ->  One byte data" +
                "\n\t255 , 128 , 140, -> Two or more byte data" +
                "\n\t120, 144 , 189   -> The 189 will ignore cause has no comma (,)";
            MessageBox.Show(message, "Warning", MessageBoxButtons.OK);

        }

        private void btnLed1Open_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                btnSystemStatusLed1.Enabled = true;
                // 0x02, 0x04, 0x07 verilerini içeren byte arrayini oluştur
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x04, 0x2C, 0x0B, 0x01, 0x2C, 0x21 }; // doğru crc değerleri 0x57, 0xF1 

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                //Etiket durumu değiştir
                lblLed1Status.Text = "ON";

                btnLed1Status.BackColor = Color.DarkGreen;

                btnSystemStatusLed1.BackColor = Color.Green;
            }
        }

        private void btnLed1Close_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                btnSystemStatusLed1.Enabled = true;
                // 0x02, 0x04, 0x07 verilerini içeren byte arrayini oluştur
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x04, 0x2C, 0x0B, 0x00, 0x2C, 0x21, }; //0x97, 0xA0 doğru crc değerleri

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                //Etiket durumu değiştir
                lblLed1Status.Text = "OFF";

                btnLed1Status.BackColor = Color.Red;

                btnSystemStatusLed1.BackColor = Color.Red;
            }
        }

        private void btnLed2Open_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                btnSystemStatusLed2.Enabled = true;
                // 0x02, 0x04, 0x07 verilerini içeren byte arrayini oluştur
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x05, 0x2C, 0x0C, 0x01, 0x2C, 0x21 };    //F2,F1 doğru crc değeri

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                //Etiket durumu değiştir
                lblStatusLed2.Text = "ON";

                btnLed2Status.BackColor = Color.DarkGreen;

                btnSystemStatusLed2.BackColor = Color.Green;
            }
        }

        private void btnLed2Close_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                btnSystemStatusLed2.Enabled = true;
                // 0x02, 0x04, 0x07 verilerini içeren byte arrayini oluştur
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x05, 0x2C, 0x0C, 0x00, 0x2C, 0x21 }; //0x32, 0xA0 doğru crc değeri

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                //Etiket durumu değiştir
                lblStatusLed2.Text = "OFF";

                btnLed2Status.BackColor = Color.Red;

                btnSystemStatusLed2.BackColor = Color.Red;
            }
        }

        private void btnOpenBuzzer_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                btnSystemStatusBuzzer.Enabled = true;
                // 0x02, 0x04, 0x07 verilerini içeren byte arrayini oluştur
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x06, 0x2C, 0x0D, 0x01, 0x2C, 0x21 };

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                //Etiket durumu değiştir
                lblStatusBuzzer.Text = "ON";

                btnBuzzerStatus.BackColor = Color.DarkGreen;

                btnSystemStatusBuzzer.BackColor = Color.DarkGreen;
            }
        }

        private void btnCloseBuzzer_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                btnSystemStatusBuzzer.Enabled = true;
                //0x02, 0x04, 0x07 verilerini içeren byte arrayini oluştur
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x06, 0x2C, 0x0D, 0x00, 0x2C, 0x21 };

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                //Etiket durumu değiştir
                lblStatusBuzzer.Text = "OFF";

                btnBuzzerStatus.BackColor = Color.Red;

                btnSystemStatusBuzzer.BackColor = Color.Red;
            }
        }

        private void btnSpeakerOpen_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                btnSystemStatusSpeaker.Enabled = true;

                btnSpeakerOpen.Enabled = true;
                // 0x02, 0x04, 0x07 verilerini içeren byte arrayini oluştur
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x01, 0x2C, 0x0A, 0x01, 0x2C, 0x21 };

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                //Etiket durumu değiştir
                lblStatusSpeaker.Text = "ON";

                btnSpeakerStatus.BackColor = Color.DarkGreen;

                btnSystemStatusSpeaker.BackColor = Color.Green;
            }
        }

        private void btnSpeakerClose_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                btnSystemStatusSpeaker.Enabled = true;

                btnSpeakerOpen.Enabled = true;
                // 0x02, 0x04, 0x07 verilerini içeren byte arrayini oluştur
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x01, 0x2C, 0x0A, 0x00, 0x2C, 0x21 };

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                //Etiket durumu değiştir
                lblStatusSpeaker.Text = "OFF";

                btnSpeakerStatus.BackColor = Color.Red;

                btnSystemStatusSpeaker.BackColor = Color.Red;
            }
        }

        private void btnStopMode_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                btnSystemStatusMotorManuel.BackColor = SystemColors.Control;
                btnSystemStatusMotorAuto.BackColor = SystemColors.Control;

                btnStopMode.Enabled = true;
                // 0x02, 0x04, 0x07 verilerini içeren byte arrayini oluştur
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x07, 0x2C, 0x0D, 0x01, 0x2C, 0x21 };

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                //Etiket durumu değiştir
                lblStatusMotor.Text = "STOP";
                btnAutoMode.Enabled = true;
                btnManuelMode.Enabled = true;
                btnSystemStatusMotorStop.BackColor = Color.Green;
            }
        }

        private void btnManuelMode_Click(object sender, EventArgs e)
        {
            if ((serialPort1.IsOpen) && (tBoxMotorRight.TextLength > 0) && (tBoxMotorLeft.TextLength > 0) && (tBoxTurnSpeed.TextLength > 0) && (IsTextBoxNumeric(tBoxMotorRight.Text) == true) && (IsTextBoxNumeric(tBoxMotorLeft.Text) == true) && (IsTextBoxNumeric(tBoxTurnSpeed.Text) == true))
            {
                btnSystemStatusMotorStop.BackColor = SystemColors.Control;
                btnSystemStatusMotorAuto.BackColor = SystemColors.Control;
                btnStopMode.Enabled = true;

                string motorTextLeftValue = tBoxMotorLeft.Text;
                string motorTextRightValue = tBoxMotorRight.Text;
                string motorTextTurnSpeed = tBoxTurnSpeed.Text;

                byte motorRightByte = (byte)(Convert.ToByte(motorTextRightValue, 10));
                byte motorLeftByte = (byte)(Convert.ToByte(motorTextLeftValue, 10));
                byte motorSpeedByte = (byte)(Convert.ToByte(motorTextTurnSpeed, 10));

                byte combinedMotorByte = (byte)((motorRightByte << 4) | motorLeftByte);

                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x3D, 0x2C, motorSpeedByte, combinedMotorByte, 0x2C, 0x21 };

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                btnAutoMode.Enabled = true;
                btnManuelMode.Enabled = true;
                lblStatusMotor.Text = "MANUEL";
                btnSystemStatusMotorManuel.BackColor = Color.Green;
            }
            else
            {
                if (tBoxMotorRight.TextLength == 0)
                {
                    tBoxMotorRight.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (tBoxMotorLeft.TextLength == 0)
                {
                    tBoxMotorLeft.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (tBoxTurnSpeed.TextLength == 0)
                {
                    tBoxTurnSpeed.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (IsTextBoxNumeric(tBoxMotorRight.Text) == false)
                {
                    tBoxMotorRight.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (IsTextBoxNumeric(tBoxMotorLeft.Text) == false)
                {
                    tBoxMotorLeft.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (IsTextBoxNumeric(tBoxTurnSpeed.Text) == false)
                {
                    tBoxTurnSpeed.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void btnAutoMode_Click(object sender, EventArgs e)
        {
            if ((serialPort1.IsOpen) && (tBoxMotorRight.TextLength > 0) && (tBoxMotorLeft.TextLength > 0) && (tBoxTurnSpeed.TextLength > 0) && (IsTextBoxNumeric(tBoxMotorRight.Text) == true) && (IsTextBoxNumeric(tBoxMotorLeft.Text) == true) && (IsTextBoxNumeric(tBoxTurnSpeed.Text) == true))
            {
                btnSystemStatusMotorStop.BackColor = SystemColors.Control;
                btnSystemStatusMotorManuel.BackColor = SystemColors.Control;

                btnAutoMode.Enabled = false;
                btnManuelMode.Enabled = false;

                string motorTextLeftValue = tBoxMotorLeft.Text;
                string motorTextRightValue = tBoxMotorRight.Text;
                string motorTextTurnSpeed = tBoxTurnSpeed.Text;

                if (motorTextLeftValue != motorTextRightValue)
                {
                    MessageBox.Show("In automatic mode the number of right and left turns cannot be the same", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    btnAutoMode.Enabled = true;
                    btnManuelMode.Enabled = true;
                }
                else
                {
                    byte motorRightByte = (byte)(Convert.ToByte(motorTextRightValue, 10));
                    byte motorLeftByte = (byte)(Convert.ToByte(motorTextLeftValue, 10));
                    byte motorSpeedByte = (byte)(Convert.ToByte(motorTextTurnSpeed, 10));

                    byte combinedMotorByte = (byte)((motorRightByte << 4) | motorLeftByte);

                    byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x4F, 0x2C, motorSpeedByte, combinedMotorByte, 0x2C, 0x21 };

                    Crc16();

                    ushort crc_val = ComputeChecksum(dataToSend);

                    // CRC değerini byte dizisine ekle
                    byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                    Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                    dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                    dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                    // Seri port üzerinden veriyi gönder
                    serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                    lblStatusMotor.Text = "AUTO";

                    btnSystemStatusMotorAuto.BackColor = Color.Green;
                }
            }
            else
            {
                if (tBoxMotorRight.TextLength == 0)
                {
                    tBoxMotorRight.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (tBoxMotorLeft.TextLength == 0)
                {
                    tBoxMotorLeft.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (tBoxTurnSpeed.TextLength == 0)
                {
                    tBoxTurnSpeed.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (IsTextBoxNumeric(tBoxMotorRight.Text) == false)
                {
                    tBoxMotorRight.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (IsTextBoxNumeric(tBoxMotorLeft.Text) == false)
                {
                    tBoxMotorLeft.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (IsTextBoxNumeric(tBoxTurnSpeed.Text) == false)
                {
                    tBoxTurnSpeed.Text = "0";
                    MessageBox.Show("Please enter a valid value !!!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                serialPort1.PortName = cBoxCOMPORT.Text;
                serialPort1.BaudRate = Convert.ToInt32(cBoxBAUDRATE.Text);
                serialPort1.DataBits = Convert.ToInt32(cBoxDATABITS.Text);
                serialPort1.StopBits = (StopBits)Enum.Parse(typeof(StopBits), cBoxSTOPBITS.Text);
                serialPort1.Parity = (Parity)Enum.Parse(typeof(Parity), cBoxPARITYBITS.Text);

                serialPort1.Open();
                progressBar1.Value = 100;
                lblStatusCom.Text = "ON";
            }
            catch (Exception err)
            {
                MessageBox.Show(err.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                lblSystemTestStatus.Text = "NO TEST STARTED";
                btnLed1Status.BackColor = SystemColors.Control;
                btnLed2Status.BackColor = SystemColors.Control;
                btnBuzzerStatus.BackColor = SystemColors.Control;
                btnSpeakerStatus.BackColor = SystemColors.Control;
                btnSystemStatusLed1.BackColor = SystemColors.Control;
                btnSystemStatusLed2.BackColor = SystemColors.Control;
                btnSystemStatusBuzzer.BackColor = SystemColors.Control;
                btnSystemStatusSpeaker.BackColor = SystemColors.Control;
                btnSystemStatusMotorStop.BackColor = SystemColors.Control;
                btnSystemStatusMotorAuto.BackColor = SystemColors.Control;
                btnSystemStatusMotorStop.BackColor = SystemColors.Control;
                serialPort1.Close();
                progressBar1.Value = 0;
                lblStatusCom.Text = "OFF";
            }
        }

        private void chBoxDtrEnable_CheckedChanged_1(object sender, EventArgs e)
        {//CheckedChanged yöntemi, chechbox öncekinden farklı bir duruma geldiğinde yürütülür. checked to be Unchecked  or Unchecked to be checked
            if (chBoxDtrEnable.Checked)
            {
                serialPort1.DtrEnable = true;           //Active DTR for serial port 1  
                MessageBox.Show("DTR Enable", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning); //This is just an addition
            }
            else { serialPort1.DtrEnable = false; }     //Dective DTR for serial port 1
        }

        private void chBoxRtsEnable_CheckedChanged_1(object sender, EventArgs e)
        {//CheckedChanged yöntemi, chechbox öncekinden farklı bir duruma geldiğinde yürütülür. checked to be Unchecked  or Unchecked to be checked
            if (chBoxRtsEnable.Checked)
            {
                serialPort1.RtsEnable = true;           //Active RTS for serial port 1
                MessageBox.Show("RTS Enable", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning); //This is just an addition
            }
            else { serialPort1.RtsEnable = false; }     //Dective RTS for serial port 1
        }

        private void button4_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                tBoxDataIN.Text += Environment.NewLine;
            }
        }

        private void btnClear_Click_1(object sender, EventArgs e)
        {
            if (tBoxDataIN.Text != "")
            {
                tBoxDataIN.Text = "";
                tBoxRxLength.Text = "0";
            }
        }

        private void tBoxMotorRight_TextChanged(object sender, EventArgs e)
        {
            int right;
            Int32.TryParse(tBoxMotorRight.Text, out right);

            if (right > 15)
            {
                tBoxMotorRight.Text = "0";
                MessageBox.Show("Maximum right turn limit (0-15) , enter value again !!!", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            else if ((right == 36) || (right == 44) || (right == 33)) //yasaklı karakterler $(36) ,(44) !(33)
            {
                tBoxMotorRight.Text = "0";
                MessageBox.Show("Forbidden characters $(36) ,(44) !(33) are not allowed!!!", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void tBoxMotorLeft_TextChanged(object sender, EventArgs e)
        {
            int left;
            Int32.TryParse(tBoxMotorLeft.Text, out left);

            if (left > 15)
            {
                tBoxMotorLeft.Text = "0";
                MessageBox.Show("Maximum right turn limit (0-15) , enter value again !!!", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            else if ((left == 36) || (left == 44) || (left == 33)) //yasaklı karakterler $(36) ,(44) !(33)
            {
                tBoxMotorLeft.Text = "0";
                MessageBox.Show("Forbidden characters $(36) ,(44) !(33) are not allowed!!!", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void pictureBox1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            string url = "https://www.unisign.com.tr/";

            // Varsayılan tarayıcıda URL'yi aç
            try
            {
                System.Diagnostics.Process.Start(url);
            }
            catch (Exception ex)
            {
                MessageBox.Show("URL açılamadı: " + ex.Message);
            }
        }

        private void label23_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            string url = "https://www.linkedin.com/in/kubilay-k%C3%B6zleme-533872206/";

            // Varsayılan tarayıcıda URL'yi aç
            try
            {
                System.Diagnostics.Process.Start(url);
            }
            catch (Exception ex)
            {
                MessageBox.Show("URL açılamadı: " + ex.Message);
            }
        }

        private ushort ComputeChecksum(byte[] bytes)
        {
            ushort crc = 0;
            for (int i = 0; i < bytes.Length; ++i)
            {
                byte index = (byte)(crc ^ bytes[i]);
                crc = (ushort)((crc >> 8) ^ table[index]);
            }
            return crc;
        }

        private byte[] ComputeChecksumBytes(byte[] bytes)
        {
            ushort crc = ComputeChecksum(bytes);
            return BitConverter.GetBytes(crc);
        }

        private void Crc16()
        {
            ushort value;
            ushort temp;
            for (ushort i = 0; i < table.Length; ++i)
            {
                value = 0;
                temp = i;
                for (byte j = 0; j < 8; ++j)
                {
                    if (((value ^ temp) & 0x0001) != 0)
                    {
                        value = (ushort)((value >> 1) ^ polynomial);
                    }
                    else
                    {
                        value >>= 1;
                    }
                    temp >>= 1;
                }
                table[i] = value;
            }
        }

        private void tBoxTurnSpeed_TextChanged(object sender, EventArgs e)
        {
            int speed;
            Int32.TryParse(tBoxTurnSpeed.Text, out speed);

            if (speed > 255)
            {
                tBoxTurnSpeed.Text = "0";
                MessageBox.Show("Maximum turn speed limit (0-255) , enter value again !!!", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            else if ((speed == 36) || (speed == 44) || (speed == 33)) //yasaklı karakterler $(36) ,(44) !(33)
            {
                tBoxTurnSpeed.Text = "0";
                MessageBox.Show("Forbidden characters $(36) ,(44) !(33) are not allowed!!!", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void tBoxTurnSpeed_MouseClick(object sender, MouseEventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                if (tBoxTurnSpeed.Text == "(0-255)")
                {
                    tBoxTurnSpeed.Text = "0";
                }
            }
        }

        private void tBoxMotorRight_MouseClick(object sender, MouseEventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                if (tBoxMotorRight.Text == "(0-15)")
                {
                    tBoxMotorRight.Text = "0";
                }
            }
        }

        private void tBoxMotorLeft_MouseClick(object sender, MouseEventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                if (tBoxMotorLeft.Text == "(0-15)")
                {
                    tBoxMotorLeft.Text = "0";
                }
            }
        }

        public bool IsTextBoxNumeric(string textBox)
        {
            foreach (char c in textBox)
            {
                if (!char.IsDigit(c))
                {
                    return false;
                }
            }
            return true;
        }

        private void pictureBox5_MouseClick(object sender, MouseEventArgs e)
        {
            string url = "http://odetron.com/";

            // Varsayılan tarayıcıda URL'yi aç
            try
            {
                System.Diagnostics.Process.Start(url);
            }
            catch (Exception ex)
            {
                MessageBox.Show("URL açılamadı: " + ex.Message);
            }
        }

        private void btnMotorControlInfo_MouseClick(object sender, MouseEventArgs e)
        {
            string message = "If you want to use the motor control in the desired way, please follow the instructions below. \n" +
                             "\nThe number of forward or reverse rotation of the motor should be maximum 15.\n" +
                             "\nThe forward or reverse rotation time of the motor (max 255 must be entered) is calculated as one revolution.\n" +
                             "\nMaximum one revolution time is 10 seconds and minimum 10ms.And this process is calculated with the software in the processor after the mode of the motor is set and displayed on the monitor.\n" +
                             "\nThe user does not need to calculate the rotation time of the motor, he can set the time with the desired precision by sending a command and watching it on the monitor.\n";
            MessageBox.Show(message, "Info", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void btnSystemTestStart_MouseClick(object sender, MouseEventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x20, 0x2C, 0x2E, 0x25, 0x2C, 0x21 }; //0xB4, 0xBC doğru crc değerleri

                Crc16();

                ushort crc_val = ComputeChecksum(dataToSend);

                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı

                // Seri port üzerinden veriyi gönder
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);

                if (radioButtLedTest.Checked)
                {
                    dataToSend = new byte[] { 0x24, 0x2C, 0x08, 0x2C, 0x10, 0x01, 0x2C, 0x21 }; //0x7F, 0xF7 doğru crc değerleri
                    Crc16();
                    crc_val = ComputeChecksum(dataToSend);
                    // CRC değerini byte dizisine ekle
                    dataWithCrc = new byte[dataToSend.Length + 2];
                    Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                    dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                    dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı
                    // Seri port üzerinden veriyi gönder
                    serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);
                    lblSystemTestStatus.Text = "TESTING LED";
                }
                else if (radioButtBuzzerTest.Checked)
                {
                    dataToSend = new byte[] { 0x24, 0x2C, 0x09, 0x2C, 0x11, 0x02, 0x2C, 0x21 }; //0x52, 0x07 doğru crc değerleri
                    Crc16();
                    crc_val = ComputeChecksum(dataToSend);
                    // CRC değerini byte dizisine ekle
                    dataWithCrc = new byte[dataToSend.Length + 2];
                    Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                    dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                    dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı
                    // Seri port üzerinden veriyi gönder
                    serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);
                    lblSystemTestStatus.Text = "TESTING BUZZER";
                }
                else if (radioButtSpeakerTest.Checked)
                {
                    dataToSend = new byte[] { 0x24, 0x2C, 0x0A, 0x2C, 0x12, 0x03, 0x2C, 0x21 }; //0xE5, 0xE6 doğru crc değerleri
                    Crc16();
                    crc_val = ComputeChecksum(dataToSend);
                    // CRC değerini byte dizisine ekle
                    dataWithCrc = new byte[dataToSend.Length + 2];
                    Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                    dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                    dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı
                    // Seri port üzerinden veriyi gönder
                    serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);
                    lblSystemTestStatus.Text = "TESTING SPEAKER";
                }
                else if (radioButtMotorTest.Checked)
                {
                    dataToSend = new byte[] { 0x24, 0x2C, 0x0B, 0x2C, 0x13, 0x04, 0x2C, 0x21 }; //0x09, 0xE7 doğru crc değerleri
                    Crc16();
                    crc_val = ComputeChecksum(dataToSend);
                    // CRC değerini byte dizisine ekle
                    dataWithCrc = new byte[dataToSend.Length + 2];
                    Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                    dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                    dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı
                    // Seri port üzerinden veriyi gönder
                    serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);
                    lblSystemTestStatus.Text = "TESTING MOTOR";
                }
            }
        }

        private void btnSystemTestStop_MouseClick(object sender, MouseEventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                byte[] dataToSend = new byte[] { 0x24, 0x2C, 0x22, 0x2C, 0x2F, 0x26, 0x2C, 0x21 }; //0xAA, 0x4C doğru crc değerleri
                Crc16();
                ushort crc_val = ComputeChecksum(dataToSend);
                // CRC değerini byte dizisine ekle
                byte[] dataWithCrc = new byte[dataToSend.Length + 2];
                Array.Copy(dataToSend, dataWithCrc, dataToSend.Length);
                dataWithCrc[dataToSend.Length] = (byte)((crc_val >> 8) & 0xFF);  // CRC'nin yüksek byte'ı
                dataWithCrc[dataToSend.Length + 1] = (byte)(crc_val & 0xFF);        // CRC'nin düşük byte'ı
                serialPort1.Write(dataWithCrc, 0, dataWithCrc.Length);
                lblSystemTestStatus.Text = "TESTING STOP";
            }
        }

        private void btnSystemTestInfo_MouseClick(object sender, MouseEventArgs e)
        {
            string message = "A description of the tests performed to verify that the system works is given below: \n" +
                  "\nLed Test : In the led test, all leds toggle 5 times. If this operation is not performed, ‘system led test failed’ appears on the monitor, if the test is successful, ‘system led test successful’, if the stop button is pressed while the test is in progress, ‘system led test stopped’ appears.\n" +
                  "\nBuzzer Test : In the buzzer test, the buzzer toggles 3 times with a period of 300ms. If this operation is not performed, ‘system buzzer test failed’ appears on the monitor, if the test is successful, ‘system buzzer test successful’ appears, if the stop button is pressed while the test is in progress, ‘system buzzer test stopped’ appears.\n" +
                  "\nSpeaker Test : In the speaker test, the speaker is switched off for the first 3 seconds and is active for 10 seconds after switching off. If this operation is not performed, ‘system speaker test failed’ appears on the monitor, if the test is successful, ‘system speaker test successful’ appears, if the stop button is pressed while the test is in progress, ‘system speaker test stopped’ appears.\n" +
                  "\nMotor Test : In the engine test, the engine runs in forward direction for 5 seconds and in backward direction for 5 seconds. If this operation is not performed, ‘system motor test failed’ appears on the monitor, if the test is successful, ‘system motor test successful’ appears, if the stop button is pressed while the test is in progress, ‘system motor test stopped’ appears.\n" +
                  "\nWhile the system test is in progress, if the user clicks on any command button (led-1 enable, motor auto etc.), the system does not respond to it, user commands start to work after the test is over.\n" +
                  "\nBefore clicking the Start system test button, if the user has sent a command, this command is not processed and the system resets itself to idle state before the system test.While the system test is in progress, if the user clicks on any command button (led-1 enable, motor auto etc.), the system does not respond to it, user commands start to work after the test is over.\n\n\n\n" +
                  "Written by Kubilay Közleme to inform the user about system testing.";
            MessageBox.Show(message, "Info", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
    }
}
