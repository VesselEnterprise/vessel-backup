<?php

namespace App;
namespace App\Http\Controllers;

use App\Auth;
use App\User;
use App\Role;
use App\AppSettingUser;
use Illuminate\Support\Facades\DB;
use Illuminate\Http\Request;
use Spatie\BinaryUuid\HasBinaryUuid;
use Illuminate\Support\Facades\Hash;
use Illuminate\Support\Facades\Validator;

class UserController extends Controller
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

				$users = User::paginate(25);  //DB::table('users')->paginate(1);

				return view('user.list', ['users' => $users]);
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
        return view('user.create');
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {
        $user = new User();
				$user->email = $request->input('email');
				$user->user_name = $request->input('user_name');
				$user->first_name = $request->input('first_name');
				$user->last_name = $request->input('last_name');
				$user->password = Hash::make( $request->input('password') );
				$user->title = $request->input('title');
				$user->office = $request->input('office');
				$user->mobile = $request->input('mobile');
				$user->address = $request->input('address');
				$user->city = $request->input('city');
				$user->state = $request->input('state');
				$user->zip = $request->input('zip');
				$user->save();

				//Manually set to incrementing id - binary uuid side effect :()
				//$user->id = $user->getLastInsertId();

				//Assign the default user role
				$user->roles()->attach( Role::where('name', 'user')->first() );

				//Generate an API token
				$token = $user->createToken('Vessel API');
				$user->api_token = $token->accessToken;
				$user->save();

				return $this->index()->with(['success' => 'User ' . $user->email . ' was created successfully']);

    }

    /**
     * Display the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function show($id)
    {
				$user = User::withUuid($id)->first();
				$userSettings = $user->settings;

        return view('user.show', ['user' => $user, 'userSettings' => $userSettings]);
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
        //
    }

    /**
     * Remove the specified resource from storage.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function destroy($id)
    {
        //
    }
}
