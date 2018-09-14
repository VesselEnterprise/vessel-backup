<?php

namespace App\Http\Controllers;

use App;
use Illuminate\Http\Request;

class FileIgnoreTypeController extends Controller
{
    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
        $ignoreTypes = App\FileIgnoreType::paginate(25);
				return view('file.ignore_types.list', ['ignoreTypes' => $ignoreTypes]);
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
        return view('file.ignore_types.create');
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {
			$ignoreType = new App\FileIgnoreType;
			$ignoreType->directory = $request->input('file_type');
			$ignoreType->save();
			return $this->index()->with(['success' => 'Ignore type was added successfully']);
    }

    /**
     * Display the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function show($id)
    {
        $ignoreType = App\FileIgnoreType::find($id);
				return view('file.ignore_types.show', ['ignoreType' => $ignoreType]);
    }

    /**
     * Show the form for editing the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function edit($id)
    {
			return $this->show($id);
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
        $ignoreType = App\FileIgnoreType::find($id);
				$ignoreType->file_type = $request->input('file_type');
				$ignoreType->save();
				return $this->index()->with(['success' => 'File ignore type was updated successfully']);
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

		public function destroyMultiple(Request $request) {

		}
}
