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

/** Storage Provider Routes **/
Route::resource('storage', 'StorageController');
Route::post('/storage/deletes', 'StorageController@destroyMany')->name('storage.provider.destroyMany');

/** AppClient Routes **/
Route::resource('client', 'AppClientController');

/** Settings Routes **/
Route::resource('setting', 'SettingController');
Route::post('/setting/all', 'SettingController@updateAll')->name('setting.updateAll');
