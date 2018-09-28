<?php

namespace App;
namespace App\Http\Controllers;

use App\StorageProvider;
use Illuminate\Http\Request;

class StorageController extends Controller
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
        $providers = StorageProvider::paginate(5);

				return view('storage.list', ['providers' => $providers]);
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
        return view('storage.create');
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {

			$provider = new StorageProvider();
			$provider->provider_type = $request->input('provider_type');
			$provider->provider_name = $request->input('provider_name');
			$provider->bucket_name = $request->input('bucket_name');
			$provider->server = $request->input('server');
			$provider->access_id = $request->input('access_id');
			$provider->access_key = $request->input('access_key');
			$provider->storage_path = $request->input('storage_path');
			$provider->region = $request->input('region');
			$provider->priority = $request->input('priority');
			$provider->description = $request->input('description');
			$provider->save();

			return $this->index()->with(['success' => $provider->provider_name . ' was successfully added']);

    }

    /**
     * Display the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function show($id)
    {
				$provider = StorageProvider::withUuid($id)->first();
        return view('storage.show', ['provider' => $provider]);
    }

    /**
     * Show the form for editing the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function edit($id)
    {
			$provider = StorageProvider::withUuid($id)->first();
			return view('storage.show', ['provider' => $provider]);
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
        $provider = StorageProvider::withUuid($id)->first();
				$provider->provider_type = $request->input('provider_type');
				$provider->provider_name = $request->input('provider_name');
				$provider->bucket_name = $request->input('bucket_name');
				$provider->server = $request->input('server');
				$provider->access_id = $request->input('access_id');
				$provider->access_key = $request->input('access_key');
				$provider->storage_path = $request->input('storage_path');
				$provider->region = $request->input('region');
				$provider->priority = $request->input('priority');
				$provider->description = $request->input('description');
				$provider->active = filter_var($request->input('active'), FILTER_VALIDATE_BOOLEAN);
				$provider->save();

				return $this->index()->with(['success' => 'Storage Provider ' . $provider->provider_name . ' was updated successfully']);
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

		public function destroyMultiple(Request $request)
		{
			if ( empty($request->input('selectedIds')) )
			{
				return $this->index()->with(['error' => 'There were no providers selected']);
			}

			$selectedIds = array_filter($request->input('selectedIds'));

			$providers = StorageProvider::withUuid($selectedIds)->delete();

			return $this->index()->with(['success' => 'Successfully deleted ' . count($selectedIds) . ' storage provider(s)']);

		}
}
