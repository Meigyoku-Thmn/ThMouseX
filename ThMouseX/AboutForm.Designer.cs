namespace ThMouseX
{
    partial class AboutForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.lblDescription1 = new System.Windows.Forms.Label();
            this.lblDescription2 = new System.Windows.Forms.Label();
            this.cmdOK = new System.Windows.Forms.Button();
            this.cmdQuit = new System.Windows.Forms.Button();
            this.imgMouse = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.imgMouse)).BeginInit();
            this.SuspendLayout();
            // 
            // lblDescription1
            // 
            this.lblDescription1.AutoSize = true;
            this.lblDescription1.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.lblDescription1.Location = new System.Drawing.Point(74, 28);
            this.lblDescription1.Name = "lblDescription1";
            this.lblDescription1.Size = new System.Drawing.Size(268, 32);
            this.lblDescription1.TabIndex = 0;
            this.lblDescription1.Text = "ThMouseX Version 2.0.2 by Meigyoku Thmn\nA fork from ThMouse Version 0.31 of HWei";
            // 
            // lblDescription2
            // 
            this.lblDescription2.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.lblDescription2.Location = new System.Drawing.Point(12, 84);
            this.lblDescription2.Name = "lblDescription2";
            this.lblDescription2.Size = new System.Drawing.Size(362, 198);
            this.lblDescription2.TabIndex = 2;
            this.lblDescription2.Text = "How to use:\nRun a game. If it is supported by ThMouseX, a cross cursor will show." +
    "\n\nMouse move = move\nMouse left button = bomb\nMouse right button = turn on/off mo" +
    "use control.";
            // 
            // cmdOK
            // 
            this.cmdOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cmdOK.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.cmdOK.Location = new System.Drawing.Point(273, 212);
            this.cmdOK.Name = "cmdOK";
            this.cmdOK.Size = new System.Drawing.Size(88, 28);
            this.cmdOK.TabIndex = 3;
            this.cmdOK.Text = "OK";
            this.cmdOK.UseVisualStyleBackColor = true;
            this.cmdOK.Click += new System.EventHandler(this.cmdOK_Click);
            // 
            // cmdQuit
            // 
            this.cmdQuit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cmdQuit.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.cmdQuit.Location = new System.Drawing.Point(273, 242);
            this.cmdQuit.Name = "cmdQuit";
            this.cmdQuit.Size = new System.Drawing.Size(88, 28);
            this.cmdQuit.TabIndex = 4;
            this.cmdQuit.Text = "Quit";
            this.cmdQuit.UseVisualStyleBackColor = true;
            this.cmdQuit.Click += new System.EventHandler(this.cmdQuit_Click);
            // 
            // imgMouse
            // 
            this.imgMouse.Location = new System.Drawing.Point(25, 28);
            this.imgMouse.Name = "imgMouse";
            this.imgMouse.Size = new System.Drawing.Size(49, 48);
            this.imgMouse.TabIndex = 5;
            this.imgMouse.TabStop = false;
            // 
            // AboutForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(373, 284);
            this.Controls.Add(this.imgMouse);
            this.Controls.Add(this.cmdQuit);
            this.Controls.Add(this.cmdOK);
            this.Controls.Add(this.lblDescription2);
            this.Controls.Add(this.lblDescription1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AboutForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "About ThMouseX";
            this.TopMost = true;
            this.Shown += new System.EventHandler(this.AboutForm_Shown);
            ((System.ComponentModel.ISupportInitialize)(this.imgMouse)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblDescription1;
        private System.Windows.Forms.Label lblDescription2;
        private System.Windows.Forms.Button cmdOK;
        private System.Windows.Forms.Button cmdQuit;
        private System.Windows.Forms.PictureBox imgMouse;
    }
}

