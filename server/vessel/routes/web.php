<?php

/*
|--------------------------------------------------------------------------
| Web Routes
|--------------------------------------------------------------------------
|
| Here is where you can register web routes for your application. These
| routes are loaded by the RouteServiceProvider within a group which
| contains the "web" middleware group. Now create something great!
|
*/

Route::get('/', 'HomeController@index')->name('home');
Route::get('/home', 'HomeController@index')->name('home');

/** Authentication Routes **/
Auth::routes();

/** User Routes **/
Route::get('/user/profile/{id}', 'UserProfileController@show')->name('user.profile')->middleware('auth');
Route::post('/user/profile/{id}/update', 'UserProfileController@update')->name('user.profile.update')->middleware('auth');
Route::resource('user', 'UserController')->middleware('auth');

/** File Routes **/
Route::resource('file', 'FileController')->middleware('auth', 'authorizeRole:admin');

/** File Ignore Type Routes **/
Route::resource('file_ignore_type', 'FileIgnoreTypeController')->middleware('auth', 'authorizeRole:admin');
Route::post('/file_ignore_type/delete', 'FileIgnoreTypeController@destroyMultiple')->name('file_ignore_type.destroyMultiple')->middleware('auth', 'authorizeRole:admin');

/** File Ignore Dir Routes **/
Route::resource('file_ignore_dir', 'FileIgnoreDirController')->middleware('auth', 'authorizeRole:admin');
Route::post('/file_ignore_dir/delete', 'FileIgnoreDirController@destroyMultiple')->name('file_ignore_dir.destroyMultiple')->middleware('auth', 'authorizeRole:admin');

/** Storage Provider Routes **/
Route::resource('storage', 'StorageController')->middleware('auth', 'authorizeRole:admin');
Route::post('/storage/delete', 'StorageController@destroyMultiple')->name('storage.provider.destroyMultiple')->middleware('auth', 'authorizeRole:admin');

/** AppClient Routes **/
Route::resource('client', 'AppClientController')->middleware('auth', 'authorizeRole:admin');
Route::post('/client/delete', 'AppClientController@destroyMultiple')->name('client.destroyMultiple')->middleware('auth', 'authorizeRole:admin');

/** Setting Routes **/
Route::resource('setting', 'SettingController')->middleware('auth', 'authorizeRole:admin');
Route::post('/setting/all', 'SettingController@updateAll')->name('setting.updateAll')->middleware('auth', 'authorizeRole:admin');

/** User Setting Routes **/
Route::resource('user_setting', 'UserSettingController')->middleware('auth', 'authorizeRole:admin');
Route::post('/user_setting/delete', 'UserSettingController@destroyMultiple')->name('user_setting.destroyMultiple')->middleware('auth', 'authorizeRole:admin');

/** User Role Routes **/
Route::resource('user_role', 'UserRoleController')->middleware('auth', 'authorizeRole:admin');
Route::post('/user_role/delete', 'UserRoleController@destroyMultiple')->name('user_role.destroyMultiple')->middleware('auth', 'authorizeRole:admin');

/** Deployment Routes **/
Route::resource('deployment', 'DeploymentController')->middleware('auth', 'authorizeRole:admin');
Route::post('/deployment/delete', 'DeploymentController@destroyMultiple')->name('deployment.destroyMultiple')->middleware('auth', 'authorizeRole:admin');

/** Search Routes **/
Route::get('/search/{str}', 'SearchController@search');
Route::post('/search/logs', 'SearchController@searchLog');

/** Monitoring Routes **/
Route::resource('log', 'AppLogController')->middleware('auth', 'authorizeRole:admin');

/** Download Routes **/
Route::get('/download/{id}', 'DownloadController@downloadFile')->middleware('auth');

/** LDAP Routes **/
Route::resource('ldap', 'LdapController')->middleware('auth', 'authorizeRole:admin');
Route::get('/ldap/{id}/import', 'LdapController@import')->name('ldap.import')->middleware('auth', 'authorizeRole:admin');
