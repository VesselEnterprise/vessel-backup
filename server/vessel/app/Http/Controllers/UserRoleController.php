<?php

namespace App\Http\Controllers;

use App;
use Illuminate\Http\Request;

class UserRoleController extends Controller
{

    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
			$userRoles = App\UserRole::paginate(25);

			return view('user.roles.list', ['userRoles' => $userRoles]);
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
				$roles = App\Role::all();
				$users = App\User::where('active', 1)->get();
        return view('user.roles.create', ['roles' => $roles, 'users' => $users]);
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {
				$userId = App\User::encodeUuid($request->input('user'));
				$roleId = $request->input('role');
        App\UserRole::updateOrCreate(['user_id' => $userId, 'role_id' => $roleId]);

				return $this->index()->with(['success' => 'User role was added successfully']);
    }

    /**
     * Display the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function show($id)
    {
        $userRole  = App\UserRole::find($id);
				$roles = App\Role::all();
				$users = App\User::where('active', 1)->get();

				return view('user.roles.show', ['userRole' => $userRole, 'roles' => $roles, 'users' => $users]);
    }

    /**
     * Show the form for editing the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function edit($id)
    {
			return $this->show();
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
			$userId = App\User::encodeUuid($request->input('user'));
			$roleId = $request->input('role');

			$userRole = App\UserRole::find($id);
			$userRole->user_id = $userId;
			$userRole->role_id = $roleId;
			$userRole->save();

			$this->enforceAdminUser();

			return $this->index()->with(['success' => 'User role was updated successfully']);
    }

    /**
     * Remove the specified resource from storage.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function destroy($id)
    {
        App\UserRole::destroy($id);

				$this->enforceAdminUser();

				return $this->index()->with(['success' => 'User role was successfully deleted']);
    }

		public function destroyMultiple(Request $request) {

			$selectedIds = $request->input('selectedIds');

			foreach( $selectedIds as $id ) {
				App\UserRole::destroy($id);
			}

			$this->enforceAdminUser();

			return $this->index()->with(['success' => 'User role(s) were successfully deleted']);

		}

		private function enforceAdminUser() {

			$admin = App\User::where('user_name', 'admin')->first();
			$adminRole = App\Role::where('name', 'admin')->first();

			//Ensure that admin user maintains the admin role no matter what
			$userRoles = App\UserRole::where(['user_id' => $admin->user_id, 'role_id' => $adminRole->id ])->get();

			if ( sizeof($userRoles) == 0 ) {
				$userRole = new App\UserRole;
				$userRole->role_id = $adminRole->id;
				$userRole->user_id = $admin->user_id;
				$userRole->save();
			}

		}

}
