<?php

use Illuminate\Http\Request;

/*
|--------------------------------------------------------------------------
| API Routes
|--------------------------------------------------------------------------
|
| Here is where you can register API routes for your application. These
| routes are loaded by the RouteServiceProvider within a group which
| is assigned the "api" middleware group. Enjoy building your API!
|
*/

Route::middleware('auth:api')->get('/user', function (Request $request) {
    return $request->user();
});

//Heartbeat
Route::post('/heartbeat', 'api\HeartbeatController@store')->middleware('verifyClientToken');

//AppClient Install
Route::post('/client/install', 'api\AppClientController@install')->middleware('verifyDeploymentKey');

//AWS S3 Uploads
Route::post('/upload/aws', 'api\AwsUploadController@initUpload')->middleware('verifyClientToken');
Route::get('/upload/aws/{id}', 'api\AwsUploadController@show')->middleware('verifyClientToken');
Route::post('/upload/aws/{id}', 'api\AwsUploadController@complete')->middleware('verifyClientToken');
Route::delete('/upload/aws/{id}', 'api\AwsUploadController@destroy')->middleware('verifyClientToken');
Route::post('/upload/aws/{id}/sign', 'api\AwsUploadController@getSigningKey')->middleware('verifyClientToken');
