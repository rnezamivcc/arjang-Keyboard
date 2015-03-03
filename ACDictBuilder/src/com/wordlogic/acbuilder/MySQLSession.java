package com.wordlogic.acbuilder;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

public class MySQLSession {
	public Connection con;
	
	public MySQLSession() {
		String url = "jdbc:mysql://localhost:3306/";
		String dbName = "ngrams";
		String driver = "com.mysql.jdbc.Driver";
		String userName = "root"; 
		String password = "";
		
		try {
		Class.forName(driver).newInstance();
		con = DriverManager.getConnection(url+dbName,userName,password);
		} catch(Exception e) {
			throw new IllegalStateException(e);
		}
	}
	
	public String getWord(int id) throws SQLException {
		PreparedStatement pst;
		ResultSet rs;
		String ret;
		
		pst = con.prepareStatement("SELECT * FROM words WHERE id = ?");
		
		pst.setInt(1, id);
		
		rs = pst.executeQuery();
		
		if(!rs.first()) {
			rs.close();
			pst.close();
			return null;
		}
		
		ret = rs.getString("word");
		
		rs.close();
		pst.close();
		
		return ret;
	}
}