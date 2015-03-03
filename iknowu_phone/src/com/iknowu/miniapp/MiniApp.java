package com.iknowu.miniapp;

import android.widget.ImageView;

public class MiniApp {
	
	private static final int ID_OFFSET = 100;
	
	private int id;
	private String packageName;
	private String className;
	private String categories;
	private ImageView smallIcon;
	private ImageView largeIcon;
	
	private String catFound;
	private String dataFound;
	
	public MiniApp() {}
	
	public void setId( int id ) {
		this.id = id;//ID_OFFSET + id;
		if (this.smallIcon != null)
			this.smallIcon.setId(id);
		if (this.largeIcon != null)
			this.largeIcon.setId(id);
	}
	
	public void setPackageName( String name ) {
		this.packageName = name;
	}
	
	public void setClassName( String name ) {
		this.className = name;
	}
	
	public void setCategories( String categories ) {
		this.categories = categories;
	}
	
	public void setSmallIcon( ImageView icon ) {
		this.smallIcon = icon;
		this.smallIcon.setId(this.id);
	}
	
	public void setLargeIcon( ImageView icon ) {
		this.largeIcon = icon;
		this.largeIcon.setId(this.id);
	}
	
	public int getId() {
		return this.id;
	}
	
	public String getPackageName() {
		return this.packageName;
	}
	
	public String getClassName() {
		return this.className;
	}
	
	public String getCategories() {
		return this.categories;
	}
	
	public ImageView getSmallIcon() {
		return this.smallIcon;
	}
	
	public ImageView getLargeIcon() {
		return this.largeIcon;
	}
	
	public void setCategoryfound(String cat) {
		this.catFound = cat;
	}
	
	public String getCategoryFound() {
		return this.catFound;
	}
	
	public void setDatafound(String cat) {
		this.dataFound = cat;
	}
	
	public String getDataFound() {
		return this.dataFound;
	}
}
