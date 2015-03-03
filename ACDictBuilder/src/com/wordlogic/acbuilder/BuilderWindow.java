package com.wordlogic.acbuilder;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.ProgressMonitor;

public class BuilderWindow extends JFrame implements ActionListener {
	private JLabel lblWordlist;
	private JTextField txtWordlist;
	private JPanel panelWordlist;
	private JButton btnWordlist;
	
	private JLabel lblAccentFile;
	private JTextField txtAccentFile;
	private JButton btnAccentFile;
	private JPanel panelAccentFile;
	
	private JPanel panelOutfile;
	private JLabel lblOutfile;
	private JTextField txtOutfile;
	private JButton btnOutfile;
	
	private JPanel panelSubmit;
	private JButton btnSubmit;
	
	private JPanel panelDB;
	private JCheckBox chkDB;
	
	private ArrayList<JComponent> inputfields;
	
	
	public BuilderWindow() {
		Container formGrid;
		
		getContentPane().setLayout(new GridLayout(7,1));
		
		formGrid = getContentPane();
		
		inputfields = new ArrayList<JComponent>();
		
		panelWordlist = new JPanel();
		panelWordlist.setLayout(new BorderLayout());
		
		lblWordlist = new JLabel("Word list:");
		txtWordlist = new JTextField(50);
		btnWordlist = new JButton("Browse...");
		
		btnWordlist.addActionListener(this);
		
		inputfields.add(txtWordlist);
		inputfields.add(btnWordlist);
		
		panelWordlist.add(lblWordlist,BorderLayout.WEST);
		panelWordlist.add(txtWordlist,BorderLayout.CENTER);
		panelWordlist.add(btnWordlist,BorderLayout.EAST);
		
		panelAccentFile = new JPanel();
		panelAccentFile.setLayout(new BorderLayout());
		
		lblAccentFile = new JLabel("Accent file:");
		txtAccentFile = new JTextField(50);
		btnAccentFile = new JButton("Browse...");
		
		btnAccentFile.addActionListener(this);
		
		inputfields.add(txtAccentFile);
		inputfields.add(btnAccentFile);
		
		panelAccentFile.add(lblAccentFile,BorderLayout.WEST);
		panelAccentFile.add(txtAccentFile,BorderLayout.CENTER);
		panelAccentFile.add(btnAccentFile,BorderLayout.EAST);
		
		panelDB = new JPanel();
		panelDB.setLayout(new BorderLayout());
		
		chkDB = new JCheckBox("Use SQL database for 2-grams");
		
		inputfields.add(chkDB);
		
		panelDB.add(chkDB,BorderLayout.WEST);
		
		panelOutfile = new JPanel();
		panelOutfile.setLayout(new BorderLayout());
		
		lblOutfile = new JLabel("Output file:");
		txtOutfile = new JTextField(50);
		btnOutfile = new JButton("Browse...");
		
		btnOutfile.addActionListener(this);
		
		inputfields.add(txtOutfile);
		inputfields.add(btnOutfile);
		
		panelOutfile.add(lblOutfile,BorderLayout.WEST);
		panelOutfile.add(txtOutfile,BorderLayout.CENTER);
		panelOutfile.add(btnOutfile,BorderLayout.EAST);
		
		btnSubmit = new JButton("Build");
		
		initPanelSubmit();
		
		txtWordlist.setText("C:\\Users\\Reza\\Dropbox\\Photos\\code\\arjangkb\\dictionary_cooked\\english\\aac\\english.txt");
		txtAccentFile.setText("C:\\Users\\Reza\\Dropbox\\Photos\\code\\arjangkb\\dictionary_cooked\\english\\aac\\frenchaccents.txt");
		txtOutfile.setText("C:\\Users\\Reza\\Dropbox\\Photos\\code\\arjangkb\\dictionary_cooked\\english\\aac\\english.aac");
		
		formGrid.add(panelWordlist);
		formGrid.add(panelAccentFile);
		formGrid.add(panelDB);
		formGrid.add(new JPanel()); // spacer
		formGrid.add(panelOutfile);
		formGrid.add(new JPanel()); // spacer
		formGrid.add(panelSubmit);
		
		this.setTitle("Auto-correct dictionary builder");
        this.pack();
        
        btnSubmit.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				final DictBuilder builder = new DictBuilder();
				ProgressMonitor prgMon = new ProgressMonitor(BuilderWindow.this,"Building auto-correct dictionary file...",null,0,100);
				
				builder.setPath(DictBuilder.PATHWORDLIST,txtWordlist.getText());
				builder.setPath(DictBuilder.PATHOUTFILE,txtOutfile.getText());

				if(txtAccentFile.getText().length() > 1) {
					builder.setPath(DictBuilder.PATHACCENTS, txtAccentFile.getText());
				}
				
				builder.setFetchDB(chkDB.isSelected());
				
				builder.setProgressMonitor(prgMon);
				
				new Thread(new Runnable() {
					public void run() {
						int c;
						
						for(c = 0;c < inputfields.size();c++) {
							inputfields.get(c).setEnabled(false);
						}
						
						builder.go();
						
						for(c = 0;c < inputfields.size();c++) {
							inputfields.get(c).setEnabled(true);
						}
					}
				}).start();
			}
        });
        
        this.setResizable(false);
	}
	
	private void initPanelSubmit() {
		panelSubmit = new JPanel();
		panelSubmit.setLayout(new BorderLayout());
		
		panelSubmit.add(btnSubmit,BorderLayout.EAST);
	}


	@Override
	public void actionPerformed(ActionEvent e) {
		Object src = e.getSource();
		JFileChooser chooser;
		int ret;
		String path;
		
		chooser = new JFileChooser();
		
		if(src == btnOutfile) {
			ret = chooser.showSaveDialog(this);
		} else {
			ret = chooser.showOpenDialog(this);
		}
		
		if(ret == JFileChooser.APPROVE_OPTION) {
			path = chooser.getSelectedFile().getAbsolutePath();
			
			if(src == btnOutfile) {
				txtOutfile.setText(path);
			} else if(src == btnAccentFile) {
				txtAccentFile.setText(path);
			} else if(src == btnWordlist) {
				txtWordlist.setText(path);
			}
		}
	}
}