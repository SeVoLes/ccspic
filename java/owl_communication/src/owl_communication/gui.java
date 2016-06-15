package owl_communication;
//http://fazecast.github.io/jSerialComm/
import com.fazecast.jSerialComm.*;
import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.Color;
import java.awt.Container;
import java.awt.Dialog;
import java.awt.FileDialog;
import java.awt.FlowLayout;
import java.awt.Frame;  // Using Frame class in package java.awt
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Menu;
import java.awt.MenuBar;
import java.awt.MenuItem;
import java.awt.Panel;
import java.awt.TextField;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Properties;
import java.util.logging.FileHandler;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;
import java.awt.*;

import javax.swing.ImageIcon;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JProgressBar;




//A GUI program is written as a subclass of Frame - the top-level container
//This subclass inherits all properties from Frame, e.g., title, icon, buttons, content-pane
public class gui extends Frame {

/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	   private Label lblInfo;    // Declare a Label component 
	   private TextField tfCount; // Declare a TextField component 
	   private Button btnUpdate,btnInfoGetData,btnInfoSendData,btnLoadHex,btnLoadScript,btnExecute;   // Declare a Button component
	   private Panel infoPanel,updatePanel,scriptPanel,updateAndScriptPanel;
	   private MenuBar mbar;
	   private Menu menuSerialPort,menuSettings,menuBle,menuHelp,menuIp;
	   private MenuItem subMenuSerialPortPort,subMenuSerialPortSpeed,subMenuSerialBleName,subMenuIpAddress,subMenuIpPort,about;
	   
	   private int count = 0;     // Counter's value
	   private boolean updateState = false;
	   private boolean executeState = false;
	   
	   private String pathInterfacesConfig="etc/interfaces.cfg";
	   private String serialPort;
	   private int serialSpeed;
	   private String hexFileName,hexFilePath;
	   private Logger logger;
	   
	   private int serialWaitTime = 40; // millis
	   
	   private TextArea logtext;
	   


// Constructor to setup the GUI components
public gui() { 
	
	logger = Logger.getLogger("MyLog");  
    FileHandler fh;  

    try {  

        // This block configure the logger with handler and formatter  
        fh = new FileHandler("logs/main.log");  
        logger.addHandler(fh);
        SimpleFormatter formatter = new SimpleFormatter();  
        fh.setFormatter(formatter);  

        // the following statement is used to log any messages  
        logger.info("Programm start");  

    } catch (SecurityException e) {  
        e.printStackTrace();  
    } catch (IOException e) {  
        e.printStackTrace();  
    }  

      
	
	loadParams();
	
	logger.info("Load connection parameters");
	
	
	setLayout(new FlowLayout(FlowLayout.LEFT));
	setLayout(new GridLayout(1, 2));
	setBackground(Color.lightGray);
	
/////////////////////////////////////////////////////////////////////////////////////
	ImageIcon img = new ImageIcon("img/icon.png");
	setIconImage(img.getImage());
	
/////////////////////////////////////////////////////////////////////////////////////	
	// Create the menu bar
    mbar=new MenuBar();
    
    // Create the menu
    menuSettings=new Menu("Settings");
    
    // Create MenuItems
    menuSerialPort=new Menu("Serial Port");
    subMenuSerialPortPort = new MenuItem("Port: " + serialPort);
    subMenuSerialPortSpeed = new MenuItem("Speed: " + serialSpeed);
    // Attach menu items to submenu
    menuSerialPort.add(subMenuSerialPortPort);
    menuSerialPort.add(subMenuSerialPortSpeed);
    // Attach menu items to menu
    menuSettings.add(menuSerialPort);
    
    subMenuSerialPortSpeed.addActionListener ( new ActionListener() {
    	@Override
		public void actionPerformed(ActionEvent e) {
    		Frame window = new Frame();
    		Dialog d;
    		
    		d = new Dialog(window, "Set serial Speed", true);
    		d.setIconImage(img.getImage());
    		// Create a modal dialog


    		// Use a flow layout
    		d.setLayout( new FlowLayout() );
    		
    		TextField enterSpeed = new TextField(); // construct the TextField component

    		// Create an OK button
    		Button ok = new Button ("OK");
    		
    		ok.addActionListener ( new ActionListener()
    		{
    			public void actionPerformed( ActionEvent e )
    			{
    				// Hide dialog
    				serialSpeed=Integer.parseInt(enterSpeed.getText());
    				subMenuSerialPortSpeed.setLabel("Speed: " + serialSpeed);
    				
    				logger.info("Change serial speed - " + serialSpeed);
    				d.setVisible(false);
    			}
    		});

    		enterSpeed.setText("" + serialSpeed);
    		enterSpeed.setEditable(true);
    	    d.add(enterSpeed);
    	    
    	    d.add(ok);

    		// Show dialog
    		d.pack();
    		d.setVisible(true);
    		
 
		}
    	});
    
    
    
    subMenuSerialPortPort.addActionListener ( new ActionListener() {
    	@Override
		public void actionPerformed(ActionEvent e) {
    		Frame window = new Frame();
    		Dialog d;
    		d = new Dialog(window, "Set serial port", true);
    		d.setIconImage(img.getImage());
    		// Create a modal dialog


    		// Use a flow layout
    		d.setLayout( new FlowLayout() );
    		
    		TextField enterComport = new TextField(); // construct the TextField component

    		// Create an OK button
    		Button ok = new Button ("OK");
    		
    		ok.addActionListener ( new ActionListener()
    		{
    			public void actionPerformed( ActionEvent e )
    			{
    				// Hide dialog
    				serialPort=enterComport.getText();
    				subMenuSerialPortPort.setLabel("Port: " + serialPort);
    				logger.info("Change serial port - " + serialPort);
    				
    				d.setVisible(false);
    			}
    		});

    		enterComport.setText(serialPort);
    		enterComport.setEditable(true);
    	    d.add(enterComport);
    	    
    	    d.add(ok);

    		// Show dialog
    		d.pack();
    		d.setVisible(true);   	     
			
		}
    	});
    
    
    // Create MenuItems
    menuBle=new Menu("Bluetooth");
    subMenuSerialBleName = new MenuItem("Name: ");
    // Attach menu items to submenu
    menuBle.add(subMenuSerialBleName);
    // Attach menu items to menu
    menuSettings.add(menuBle);
    
 // Create MenuItems
    menuIp=new Menu("IP");
    subMenuIpAddress = new MenuItem("Address: ");
    subMenuIpPort = new MenuItem("Port: ");
    // Attach menu items to submenu
    menuIp.add(subMenuIpAddress);
    menuIp.add(subMenuIpPort);
    // Attach menu items to menu
    menuSettings.add(menuIp);
    
    // Attach menu to menu bar
    mbar.add(menuSettings);

 // Create the menu
    menuHelp=new Menu("Help");
 // Create MenuItems
    about=new MenuItem("About");
 // Attach submenu to menu
    menuHelp.add(about);
    
 // Attach menu to menu bar
    mbar.add(menuHelp);

	
    
    // Set menu bar to the frame
    setMenuBar(mbar);
    
 /////////////////////////////////////////////////////////////////////////////////////   
    
    infoPanel = new Panel();
    //infoPanel.setSize(500, 300);
    //GridLayout layoutInfo = new GridLayout(2,1);
    //layoutInfo.setHgap(10);
    //layoutInfo.setVgap(10);
    //infoPanel.setLayout(layoutInfo);
    
    //infoPanel.setBackground(Color.lightGray);

    
    btnInfoGetData = new Button("Get data");
    //btnInfoGetData.setEnabled(true);
    //btnInfoGetData.setSize(200, 50);
    //infoPanel.add(btnInfoGetData,BorderLayout.WEST);
	infoPanel.add(btnInfoGetData);
	
    btnInfoSendData = new Button("Send data");
    //btnInfoSendData.setEnabled(true);
    //btnInfoSendData.setSize(300, 50);
    //infoPanel.add(btnInfoSendData,BorderLayout.EAST);
    infoPanel.add(btnInfoSendData);
    
    lblInfo = new Label("Information");
    //lblInfo.setSize(200, 150);
    infoPanel.add(lblInfo);

    
    add(infoPanel);

    
    updateAndScriptPanel = new Panel();
    //infoPanel.setSize(500, 300);
    GridLayout layoutUpdAndUpd = new GridLayout(2,1);
    layoutUpdAndUpd.setHgap(10);
    layoutUpdAndUpd.setVgap(10);
    
    updateAndScriptPanel.setLayout(layoutUpdAndUpd);

    
    //updateAndScriptPanel.setBackground(Color.white);
    
    updatePanel = new Panel();
    //updatePanel.setBackground(Color.white);
    
    scriptPanel = new Panel();
    //scriptPanel.setBackground(Color.black);
    
    updateAndScriptPanel.add(updatePanel);
    updateAndScriptPanel.add(scriptPanel);
    
    btnLoadHex = new Button("Load HEX");
    btnLoadHex.addActionListener ( new openHexFile());
    btnLoadHex.setEnabled(true);
    btnLoadHex.setSize(300, 50);
    //infoPanel.add(btnInfoSendData,BorderLayout.EAST);
    updatePanel.add(btnLoadHex);
    
    btnUpdate = new Button("Update");
    btnUpdate.setEnabled(updateState);
    btnUpdate.addActionListener ( new updateHex());
    btnUpdate.setSize(200, 50);
    //infoPanel.add(btnInfoGetData,BorderLayout.WEST);
    updatePanel.add(btnUpdate);
    
    btnLoadScript = new Button("Load Script");
    btnLoadScript.setEnabled(true);
    btnLoadScript.setSize(300, 50);
    //infoPanel.add(btnInfoSendData,BorderLayout.EAST);
    scriptPanel.add(btnLoadScript);
    
    btnExecute = new Button("Execute");
    btnExecute.setEnabled(executeState);
    btnExecute.setSize(200, 50);
    //infoPanel.add(btnInfoGetData,BorderLayout.WEST);
    scriptPanel.add(btnExecute);
 
    add(updateAndScriptPanel);

    
    /*
	progressBar = new JProgressBar(0,100);
	progressBar.setValue(0);
	progressBar.setStringPainted(true);
	updatePanel.add(progressBar);
	progressBar.setVisible(true);
    */
    logtext = new TextArea("Logs... "
    	      +"Empty.",5,30);
    //logtext = new TextField("Logs... ");

    updatePanel.add(logtext);
  
    addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent we) {
        	saveParamChanges();
        	logger.info("Save parameters and programm stop");
            dispose();
         }
     });


    setTitle("OWL Configuration");  // "super" Frame sets its title
    setSize(500, 400);        // "super" Frame sets its initial window size

    setVisible(true);         // "super" Frame shows

	}

// Other methods
class btnClick implements ActionListener {

	@Override
	public void actionPerformed(ActionEvent arg0) {
		++count; // increase the counter value
		   // Display the counter value on the TextField tfCount
		   tfCount.setText(count + ""); // convert int to String
	}
	
}

public void loadParams() {
    Properties props = new Properties();
    InputStream is = null;
 
    // First try loading from the current directory
    try {
        File f = new File(pathInterfacesConfig);
        is = new FileInputStream( f );
    }
    catch ( Exception e ) { is = null; }
 
    try {
        if ( is == null ) {
            // Try loading from classpath
            is = getClass().getResourceAsStream(pathInterfacesConfig);
        }
 
        // Try loading properties from the file (if found)
        props.load( is );
    }
    catch ( Exception e ) { }
 
    serialPort = props.getProperty("SerialPort", "COM1");
    serialSpeed = new Integer(props.getProperty("SerialSpeed", "9600"));

}

public void saveParamChanges() {
    try {
        Properties props = new Properties();
        props.setProperty("SerialPort", serialPort);
        props.setProperty("SerialSpeed", ""+serialSpeed);

        File f = new File(pathInterfacesConfig);
        OutputStream out = new FileOutputStream( f );
        props.store(out, "Config file " + pathInterfacesConfig);
    }
    catch (Exception e ) {
        e.printStackTrace();
    }
}


class openHexFile implements ActionListener {

	public void actionPerformed(ActionEvent e) {
		Frame hexWindow = new Frame();
		FileDialog fdlg = new FileDialog(hexWindow,"Select HEX file",FileDialog.LOAD);
		fdlg.setFile("*.hex");
		fdlg.setVisible(true);
		hexFileName = fdlg.getFile();
		hexFilePath = fdlg.getDirectory();
		updateState = true;
		btnUpdate.setEnabled(updateState);
		if (hexFileName == null) {
			System.out.println("You cancelled the choice");
			logger.info("Save parameters and programm stop");
		}
		else
		{
			System.out.println("You chose " + hexFilePath + hexFileName);
			logger.info("Load hex file - "+ hexFilePath + hexFileName);
		}
	}
}

class updateHex implements ActionListener {
	//@SuppressWarnings("null")
	public void actionPerformed(ActionEvent e) {
		
		SerialPort comPort = SerialPort.getCommPort(serialPort);
		comPort.openPort();
		comPort.setBaudRate(serialSpeed);

		comPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_BLOCKING, serialWaitTime, 0);
		logtext.setText("");
		
		if(comPort.isOpen())
	    {
	        System.out.println("Port initialized!");
	        logtext.setText("Port initialized!");
	        logger.info("Serial port initialized - " + serialPort);
	        int lines = 0;
	        try (BufferedReader brc = new BufferedReader(new FileReader(hexFilePath + hexFileName))) {

		        	while (brc.readLine() != null) lines++;
		        	brc.close();
		    } catch (FileNotFoundException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
	        
	        int currentLines = 0;
	        //timeout not needed for event based reading
	        try (BufferedReader br = new BufferedReader(new FileReader(hexFilePath + hexFileName))) {
       	
				String line;

				while ((line = br.readLine()) != null) {
					
					currentLines++;
					logtext.setText("Load:" + ((Integer) currentLines * 100/lines) + "%");

					//progressBar.setValue((Integer) currentLines * 100/lines);

					// process the line.
					line = line + '\r';
					byte[] biteLine = line.getBytes();

					comPort.writeBytes(biteLine,line.length());

					long startTime = System.currentTimeMillis(); //fetch starting time
					while((System.currentTimeMillis()-startTime)<serialWaitTime )
					//	while( (comPort.bytesAvailable()<3) || ((System.currentTimeMillis()-startTime)<serialWaitTime) )
					{
						// Please wait...
					}
					
					if (comPort.bytesAvailable()<3)
					{
						System.out.println("Port not answer");
						logtext.setText("Port not answer");
						comPort.closePort();
						break;
					}
					
	
					byte[] newData = new byte[comPort.bytesAvailable()];

					comPort.readBytes(newData, comPort.bytesAvailable());
					
					if (newData[0]==0x13)
					{
						// XOFF
						System.out.println("XOFF");

						if (newData[1]==0x06)
						{
							//ACKLOD
							System.out.println("ACKLOD");

							
							if (newData[2]==0x11)
							{
								//XON
								System.out.println("XON");
							}
							else
							{
								System.out.println("XON - ERROR");
								logtext.setText("XON - ERROR");
								comPort.closePort();
								logger.info("XON - ERROR");
								break;
							}
							
						}
						else
						{
							System.out.println("ACKLOD - ERROR");
							logtext.setText("ACKLOD - ERROR");
							comPort.closePort();
							logger.info("ACKLOD - ERROR");
							break;
						}
						
						
					}
					else
					{
						System.out.println("XOFF - ERROR");
						logtext.setText("XOFF - ERROR");
						comPort.closePort();
						logger.info("XOFF - ERROR");
						break;
					}

				}
			} catch (FileNotFoundException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}

	    }

	    else
	    {
	    	
	        System.out.println("Port not available");
	        logger.info("Serial port not available - " + serialPort);
	    }   
        
		comPort.closePort();
		logtext.setText("Complete");
	}
					
					
}




// The entry main() method
public static void main(String[] args) {
   // Invoke the constructor (to setup the GUI) by allocating an instance
   new gui();
	}




}
