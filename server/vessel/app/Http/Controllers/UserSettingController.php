<?php

namespace App\Http\Controllers;

use App;
use App\Http\Controllers\Controller;
use Illuminate\Http\Request;

class UserSettingController extends Controller
{

		public function __construct()
		{
				$this->middleware('auth');
				$this->middleware('authorizeRole:admin');
		}

    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
			$userSettings = App\UserSetting::paginate(25);

			return view('setting.user.list', ['userSettings' => $userSettings]);
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
				$settings = App\AppSetting::all();
				$users = App\User::where('active', 1)->get();
        return view('setting.user.create', ['settings' => $settings, 'users' => $users]);
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {

        $userSetting = App\UserSetting::updateOrCreate([
					'setting_id' => $request->input('setting'),
					'user_id' => App\User::encodeUuid($request->input('user'))
				]);
				$userSetting->value = $request->input('value');
				$userSetting->save();

				return $this->index()->with(['success' => 'User setting was added successfully']);
    }

    /**
     * Display the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function show($id)
    {
				$userSetting = App\UserSetting::find($id);
				$settings = App\AppSetting::all();
				$users = App\User::where('active', 1)->get();
        return view('setting.user.show', ['userSetting' => $userSetting, 'settings' => $settings, 'users' => $users]);
    }

    /**
     * Show the form for editing the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function edit($id)
    {
        //
    }

    /**
     * Update the specified resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function update(Request $request, $id)
    {
			$userSetting = App\UserSetting::find($id);
			$userSetting->setting_id = $request->input('setting');
			$userSetting->user_id = App\User::encodeUuid( $request->input('user') );
			$userSetting->value = $request->input('value');
			$userSetting->save();

			return $this->index()->with(['success' => 'User setting updated successfully']);
    }

    /**
     * Remove the specified resource from storage.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function destroy($id)
    {
			App\UserSetting::destroy($id);
			return $this->index()->with(['success' => 'Deleted user setting successfully']);
    }

		public function destroyMultiple(Request $request) {

			$selectedIds = $request->input('selectedIds');

			foreach ( $selectedIds as $id ) {
				App\UserSetting::destroy($id);
			}

			return $this->index()->with(['success' => 'Successfully deleted user settings']);

		}

}
