<?php

namespace App\Http\Controllers;

use App;
use Illuminate\Http\Request;

class LdapController extends Controller
{
    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
        $ldapServers = App\LdapServer::paginate(10);

				return view('ldap.list', ['ldapServers' => $ldapServers]);
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
        //
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {
        //
    }

    /**
     * Display the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function show($id)
    {
        $ldapServer = App\LdapServer::findOrFail($id);
				return view('ldap.show', ['ldapServer' => $ldapServer]);
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
        $ldapServer = App\LdapServer::findOrFail($id);
				$ldapServer->name = $request->input('name');
				$ldapServer->server = $request->input('server');
				$ldapServer->port = $request->input('port');
				$ldapServer->username = $request->input('username');
				$ldapServer->dn = $request->input('dn');
				$ldapServer->type = $request->input('type');

				if ( !empty($request->input('password')) ) {
					$ldapServer->password = $request->input('password');
				}

				$ldapServer->save();

				return $this->index()->with(['success' => 'LDAP server was updated successfully']);

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

		public function import(Request $request, $id) {

			$ldapServer = App\LdapServer::findOrFail($id);
			$ldapServer->import();

		}

}
