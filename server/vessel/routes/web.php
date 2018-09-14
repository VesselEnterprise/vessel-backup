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

Route::get('/', function () {
    return view('welcome');
});

Route::get('/home', 'HomeController@index')->name('home');

/** Authentication Routes **/
Auth::routes();

/** User Routes **/
Route::get('/user/profile/{id}', 'UserProfileController@show')->name('user.profile');
Route::post('/user/profile/{id}/update', 'UserProfileController@update')->name('user.profile.update');
Route::resource('user', 'UserController');

/** File Routes **/
Route::resource('file', 'FileController');

/** File Ignore Type Routes **/
Route::resource('file_ignore_type', 'FileIgnoreTypeController');
Route::post('/file_ignore_type/delete', 'FileIgnoreTypeController@destroyMultiple')->name('file_ignore_type.destroyMultiple');

/** File Ignore Dir Routes **/
Route::resource('file_ignore_dir', 'FileIgnoreDirController');
Route::post('/file_ignore_dir/delete', 'FileIgnoreDirController@destroyMultiple')->name('file_ignore_dir.destroyMultiple');

/** Storage Provider Routes **/
Route::resource('storage', 'StorageController');
Route::post('/storage/delete', 'StorageController@destroyMultiple')->name('storage.provider.destroyMultiple');

/** AppClient Routes **/
Route::resource('client', 'AppClientController');
Route::post('/client/delete', 'AppClientController@destroyMultiple')->name('client.destroyMultiple');

/** Setting Routes **/
Route::resource('setting', 'SettingController');
Route::post('/setting/all', 'SettingController@updateAll')->name('setting.updateAll');

/** User Setting Routes **/
Route::resource('user_setting', 'UserSettingController');
Route::post('/user_setting/delete', 'UserSettingController@destroyMultiple')->name('user_setting.destroyMultiple');

/** User Role Routes **/
Route::resource('user_role', 'UserRoleController');
Route::post('/user_role/delete', 'UserRoleController@destroyMultiple')->name('user_role.destroyMultiple');

/** Deployment Routes **/
Route::resource('deployment', 'DeploymentController');
Route::post('/deployment/delete', 'DeploymentController@destroyMultiple')->name('deployment.destroyMultiple');

/** Search Routes **/
Route::get('/search/{str}', 'SearchController@search');
