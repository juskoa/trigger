package miclockGui;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Map;

import javax.swing.JFrame;
import javax.swing.JOptionPane;


/**
 * Main class of the program. Starts the program from the main method.
 * @author oerjan
 *
 */
public class Main {
	
	/**
	 * Enables the BEAM2 and REF buttons
	 */ 
	public final static boolean ENABLE_EXTRA_BUTTONS = false;
	
	/**
	 * The minimum value of the Beam Mode in BEAM1 in Auto mode.
	 * Before this value clock = LOCAL
	 */
	public final static int AUTO_BEAM_MIN = 4;
	
	/**
	 * The maximum value of the Beam Mode in BEAM1 in Auto mode. 
	 * After this value clock = LOCAL
	 */ 
	public final static int AUTO_BEAM_MAX = 12;
	
	/**
	 * The minimum Beam Mode value for when a Clock Shift is valid.
	 */
	public final static int SHIFT_VALID_MIN = 8;
	
	/**
	 * The maximum Beam Mode value for when a Clock Shift is valid.
	 */
	public final static int SHIFT_VALID_MAX = 11;
	
	/**
	 * The maximum number of lines in the log
	 */
	public final static int LOG_LENGTH = 100;
	
	/**
	 * The lower limit on when the shift can occur. Given in 10ps -> 2=20ps.
	 */
	public final static int SHIFT_LOWER_LIMIT = 2;
	
	/**
	 * The upper limit on when the shift can occur. Given in 10ps -> 2-20ps.
	 */
	public final static int SHIFT_UPPER_LIMIT = 150;
	
	public static String VMECF_DIR;
	public static String VMEWORK_DIR;
	public static String MICLOCK_ID;
	public static String PROCESS_ID;
	public static String HTML_LOCATION;
	
	/**
	 * Starts the program.
	 * @param args
	 */
	public static void main(String[] args) {
		// Reads the system enviroment
		Map<String, String> env = System.getenv();
		// If no DIM_DNS_NODE is set -> exit
		if(env.get("DIM_DNS_NODE") == null)
		{
			JOptionPane.showMessageDialog(null, "No DIM_DNS_NODE variable" +
					" set. Exiting...",
					"DIM_DNS_NODE error", JOptionPane.ERROR_MESSAGE);
			System.exit(0);
		}
		
		Main.VMECF_DIR = env.get("VMECFDIR");
		Main.VMEWORK_DIR = env.get("VMEWORKDIR");
		
		if(env.get("VMESITE").equals("ALICE"))
		{
			Main.MICLOCK_ID = "/data/dl/snapshot/alidcsvme017/home/alice/" +
			"trigger/v/vme/WORK/miclockid";
		}
		else
		{
			if(env.get("USER").equals("oerjan"))
			{
				Main.MICLOCK_ID = "/home/dl/snapshot/altri1/home/alice/" +
				"trigger/v/vme/WORK/oerjan/miclockid";
			}
			else if(env.get("USER").equals("trigger"))
			{
				Main.MICLOCK_ID = "/home/dl/snapshot/altri1/home/alice/" +
				"trigger/v/vme/WORK/miclockid";
			}
		}
		
		// Gets the process id
		Main.PROCESS_ID = java.lang.management.ManagementFactory.
		getRuntimeMXBean().getName();
		Main.PROCESS_ID = Main.PROCESS_ID.substring(
				0,Main.PROCESS_ID.indexOf('@'));
		
		Main.HTML_LOCATION = env.get("HOME") + "/CNTRRD/htmls/clockinfo";
		
		if(env.get("USER").equals("trigger") || 
				env.get("USER").equals("oerjan"))
		{
			File id = new File(Main.MICLOCK_ID);
			if(id.exists())
			{
				try {
					BufferedReader in = new BufferedReader( new FileReader(id));
					String temppid = in.readLine();
					
					String msg = "A miclock process " +
					"already exists, pid: " + temppid + "\nIf you " +
					"cannot locate window where " + temppid + "is" +
					" started, please remove miclockid file and kill " +
					"miclock process, i.e.:\n" +
					"kill "+ temppid +
					"\nrm " + Main.MICLOCK_ID +
					"\nThen start miclock again.";
					// Shows error dialog to user
					JOptionPane.showMessageDialog(null, msg,
							"MICLOCK_ALREADY_RUNNING error", 
							JOptionPane.ERROR_MESSAGE);
					in.close();
					System.exit(0);
				
				} catch (FileNotFoundException e) {
					e.printStackTrace();
					System.out.println(e.getMessage());
					
					// Shows error dialog to user
					JOptionPane.showMessageDialog(null, "Please delete " +
							"the file at\n" + Main.MICLOCK_ID
							+"\nif it exists and restart the program",
							"PIDFILE_NOT_FOUND error", 
							JOptionPane.ERROR_MESSAGE);
					System.exit(0);
				
				} catch (IOException e) {
					e.printStackTrace();
					System.out.println(e.getMessage());
					
					// Shows error dialog to user
					JOptionPane.showMessageDialog(null, "Please delete " +
							"the file at\n" + Main.MICLOCK_ID
							+"\nif it exists and restart the program",
							"PIDFILE_IO error", 
							JOptionPane.ERROR_MESSAGE);
					System.exit(0);
				}
			}
			
			try {
				BufferedWriter out = new BufferedWriter(new FileWriter(id));
				out.write(Main.PROCESS_ID+"\n");
				out.close();
			
			} catch (IOException e) {
				e.printStackTrace();
				System.out.println(e.getMessage());
				// Shows error dialog to user
				JOptionPane.showMessageDialog(null, "Writing process ID" +
						"failed. Please delete the file at\n" 
						+ Main.MICLOCK_ID + "\nif it exists and restart" +
						" the program",
						"PID_WRITE error", JOptionPane.ERROR_MESSAGE);
				System.exit(0);
			}
		}
		
		MiClockClient client = new MiClockClient();
		LogWriter logWriter = new LogWriter();
		HTMLWriter htmlWriter = new HTMLWriter(client);
		JFrame frame = new JFrame();
		
		frame.setLayout(new GridBagLayout());
		GridBagConstraints constraints = new GridBagConstraints();
		
		constraints.gridx = 0;
		constraints.gridy = 0;
		constraints.gridheight = 1;
		constraints.gridwidth = 1;
		constraints.fill = GridBagConstraints.BOTH;
		constraints.weightx = 0.5;
		constraints.weighty = 5.5;
		
		frame.setSize(800,600);
		frame.setMinimumSize(new Dimension(400, 400));
//		frame.setResizable(false);
		
		InfoPanel ip = new InfoPanel(client, logWriter);
		ButtonPanel bp = new ButtonPanel(client);
		frame.add(ip, constraints);
		
		constraints.gridy = 1;
		constraints.weighty = 0.5;
		
		frame.add(bp, constraints);
		frame.setVisible(true);
		frame.setTitle("MiclockGui");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		ExitCleaner hook = new Main.ExitCleaner(client, logWriter, htmlWriter);
		Runtime.getRuntime().addShutdownHook(hook);
		client.updateAll();
	}
	
	/**
	 * Class for handling the termination of the program. Shuts down the DimInfo
	 * objects, closes the logWriter and deletes the MICLOCKID file. This will
	 * run as its own thread at the end of the program.
	 */
	private static class ExitCleaner extends Thread {
		
		private MiClockClient client;
		private LogWriter logWriter;
		private HTMLWriter htmlWriter;
		
		/**
		 * Constructor
		 * @param client The MiClockclient for the program
		 * @param logWriter The Log Writer for the program
		 */
		private ExitCleaner(MiClockClient client, LogWriter logWriter,
				HTMLWriter htmlWriter)
		{
			this.client = client;
			this.logWriter = logWriter;
			this.htmlWriter = htmlWriter;
		}
		
		/**
		 * Runs the ExitCleaner.
		 */
		public void run() {
			this.client.shutdown();
			this.logWriter.close();
			this.htmlWriter.close();
			File id = new File(Main.MICLOCK_ID);
			id.delete();
		}
	}
}
