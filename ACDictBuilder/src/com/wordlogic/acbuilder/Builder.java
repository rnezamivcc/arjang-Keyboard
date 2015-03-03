package com.wordlogic.acbuilder;

import javax.swing.JFrame;

public class Builder extends JFrame {
	private static final long serialVersionUID = -5133196389447633495L;

	public static void main(String[] args) {
		JFrame window = new BuilderWindow();
        window.setDefaultCloseOperation(EXIT_ON_CLOSE);
        window.setVisible(true);
	}
}