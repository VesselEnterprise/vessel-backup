<?php

namespace App\Http\Controllers;

use App;
use Illuminate\Http\Request;

class SearchController extends Controller
{

		public function __construct()
		{
				$this->middleware('auth');
		}

    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
        //
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
        //
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

		public function search(Request $request, $searchText) {

			$results = [
				'users' => App\User::search($searchText)->get(),
			 	'files' => App\File::search($searchText)->get(),
			 	'settings' => App\Setting::search($searchText)->get(),
				'providers' => App\StorageProvider::search($searchText)->get(),
			 	'clients' => App\AppClient::search($searchText)->get()
			];

			foreach ( $results as $key => $arr ) {
				foreach ( $arr as $k2 => $val ) {
					$arr[$k2] = $arr[$k2]->toSearchableArray();
				}
			}

			return $results;

		}

}
