<?php

namespace App\Http\Controllers;

use Auth;
use App\User;
use App\Http\Controllers\Controller;
use Illuminate\Support\Facades\Hash;
use Illuminate\Support\Facades\Validator;
use Illuminate\Http\Request;

class UserProfileController extends Controller
{

		public function __construct()
	  {
	      $this->middleware('auth');
		}

		protected function validator(array $request)
		{
				return Validator::make($request, [
						'first_name' => 'required|string|max:255',
						'last_name' => 'required|string|max:255',
						'email' => 'required|string|email|max:255|unique:users',
						'password' => 'sometimes|required|string|min:6|confirmed',
				]);
		}

		private function authorizeAction($id)
		{
			//Make sure the user is accessing their own profile, or is an admin
			if ( Auth::user()->uuid_text != $id && !Auth::user()->hasRole('admin') )
			{
				abort(401, 'This action is not authorized');
			}
		}

    /**
     * Show the profile for the given user.
     *
     * @param  int  $id
     * @return Response
     */
    public function show($id)
    {
				//Make sure the user is accessing their own profile, or is an admin
				$this->authorizeAction($id);

        return view('user.profile', ['user' => User::withUuid($id)->first()]);
    }

		public function update($id, Request $request)
		{

			//Make sure the user is accessing their own profile, or is an admin
			$this->authorizeAction($id);

			$user = User::withUuid($id)->first();
			$user->first_name = $request['first_name'];
			$user->last_name = $request['last_name'];
			$user->email = $request['email'];
			$user->title = $request['title'];
			$user->office = $request['office'];
			$user->mobile = $request['mobile'];
			$user->address = $request['address'];
			$user->city = $request['city'];
			$user->state = $request['state'];
			$user->zip = $request['zip'];

			//Only update the password if provided
			if ( $request['password'] != '' )
			{
				$user->password = Hash::make($request['password']);
			}

			$user->save();

			//Conditionally reset password

			return view('user.profile', [
				'user' => User::withUuid($id)->first(),
				'status' => 'Profile has been updated'
			]);
		}
}

?>
