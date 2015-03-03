/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: C:\\Users\\Reza\\Dropbox\\Photos\\code\\arjangkb\\iknowu_tablet\\src\\com\\iknowu\\miniapp\\IKnowUMiniAppInterface.aidl
 */
package com.iknowu.miniapp;
public interface IKnowUMiniAppInterface extends android.os.IInterface
{
/** Local-side IPC implementation stub class. */
public static abstract class Stub extends android.os.Binder implements com.iknowu.miniapp.IKnowUMiniAppInterface
{
private static final java.lang.String DESCRIPTOR = "com.iknowu.miniapp.IKnowUMiniAppInterface";
/** Construct the stub at attach it to the interface. */
public Stub()
{
this.attachInterface(this, DESCRIPTOR);
}
/**
 * Cast an IBinder object into an com.iknowu.miniapp.IKnowUMiniAppInterface interface,
 * generating a proxy if needed.
 */
public static com.iknowu.miniapp.IKnowUMiniAppInterface asInterface(android.os.IBinder obj)
{
if ((obj==null)) {
return null;
}
android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof com.iknowu.miniapp.IKnowUMiniAppInterface))) {
return ((com.iknowu.miniapp.IKnowUMiniAppInterface)iin);
}
return new com.iknowu.miniapp.IKnowUMiniAppInterface.Stub.Proxy(obj);
}
@Override public android.os.IBinder asBinder()
{
return this;
}
@Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
{
switch (code)
{
case INTERFACE_TRANSACTION:
{
reply.writeString(DESCRIPTOR);
return true;
}
case TRANSACTION_getView:
{
data.enforceInterface(DESCRIPTOR);
java.lang.String _arg0;
_arg0 = data.readString();
java.lang.String _arg1;
_arg1 = data.readString();
java.lang.String _arg2;
_arg2 = data.readString();
android.widget.RemoteViews _result = this.getView(_arg0, _arg1, _arg2);
reply.writeNoException();
if ((_result!=null)) {
reply.writeInt(1);
_result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
}
else {
reply.writeInt(0);
}
return true;
}
case TRANSACTION_getSmallIcon:
{
data.enforceInterface(DESCRIPTOR);
android.widget.RemoteViews _result = this.getSmallIcon();
reply.writeNoException();
if ((_result!=null)) {
reply.writeInt(1);
_result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
}
else {
reply.writeInt(0);
}
return true;
}
case TRANSACTION_getLargeIcon:
{
data.enforceInterface(DESCRIPTOR);
android.widget.RemoteViews _result = this.getLargeIcon();
reply.writeNoException();
if ((_result!=null)) {
reply.writeInt(1);
_result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
}
else {
reply.writeInt(0);
}
return true;
}
case TRANSACTION_onFinishConnection:
{
data.enforceInterface(DESCRIPTOR);
this.onFinishConnection();
reply.writeNoException();
return true;
}
}
return super.onTransact(code, data, reply, flags);
}
private static class Proxy implements com.iknowu.miniapp.IKnowUMiniAppInterface
{
private android.os.IBinder mRemote;
Proxy(android.os.IBinder remote)
{
mRemote = remote;
}
@Override public android.os.IBinder asBinder()
{
return mRemote;
}
public java.lang.String getInterfaceDescriptor()
{
return DESCRIPTOR;
}
/**
     * Called by the keyboard when it needs to display the Mini-apps view
     * this is usually when a user has clicked on the apps icon in the action bar.
     */
@Override public android.widget.RemoteViews getView(java.lang.String packageName, java.lang.String param, java.lang.String category) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
android.widget.RemoteViews _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeString(packageName);
_data.writeString(param);
_data.writeString(category);
mRemote.transact(Stub.TRANSACTION_getView, _data, _reply, 0);
_reply.readException();
if ((0!=_reply.readInt())) {
_result = android.widget.RemoteViews.CREATOR.createFromParcel(_reply);
}
else {
_result = null;
}
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
@Override public android.widget.RemoteViews getSmallIcon() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
android.widget.RemoteViews _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_getSmallIcon, _data, _reply, 0);
_reply.readException();
if ((0!=_reply.readInt())) {
_result = android.widget.RemoteViews.CREATOR.createFromParcel(_reply);
}
else {
_result = null;
}
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
@Override public android.widget.RemoteViews getLargeIcon() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
android.widget.RemoteViews _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_getLargeIcon, _data, _reply, 0);
_reply.readException();
if ((0!=_reply.readInt())) {
_result = android.widget.RemoteViews.CREATOR.createFromParcel(_reply);
}
else {
_result = null;
}
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
/**
	 * Called by the keyboard when it is about to close the Mini-app
	 * you can rely on this function to be called to do any finilizations
	 */
@Override public void onFinishConnection() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_onFinishConnection, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
}
static final int TRANSACTION_getView = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
static final int TRANSACTION_getSmallIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
static final int TRANSACTION_getLargeIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
static final int TRANSACTION_onFinishConnection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
}
/**
     * Called by the keyboard when it needs to display the Mini-apps view
     * this is usually when a user has clicked on the apps icon in the action bar.
     */
public android.widget.RemoteViews getView(java.lang.String packageName, java.lang.String param, java.lang.String category) throws android.os.RemoteException;
public android.widget.RemoteViews getSmallIcon() throws android.os.RemoteException;
public android.widget.RemoteViews getLargeIcon() throws android.os.RemoteException;
/**
	 * Called by the keyboard when it is about to close the Mini-app
	 * you can rely on this function to be called to do any finilizations
	 */
public void onFinishConnection() throws android.os.RemoteException;
}
