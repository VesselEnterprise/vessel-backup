<?php

namespace App\Http\Controllers;

use App;
use Illuminate\Http\Request;

class FileIgnoreDirController extends Controller
{
    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
			$ignoreDirs = App\FileIgnoreDir::paginate(25);
			return view('file.ignore_dirs.list', ['ignoreDirs' => $ignoreDirs]);
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
        return view('file.ignore_dirs.create');
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {
        $ignoreDir = new App\FileIgnoreDir;
				$ignoreDir->directory = $request->input('directory');
				$ignoreDir->save();
				return $this->index()->with(['success' => 'Ignore directory was added successfully']);
    }

    /**
     * Display the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function show($id)
    {
			$ignoreDir = App\FileIgnoreDir::find($id);
			return view('file.ignore_dirs.show', ['ignoreDir' => $ignoreDir]);
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
        $ignoreDir = App\FileIgnoreDir::find($id);
				$ignoreDir->directory = $request->input('directory');
				$ignoreDir->save();
				return $this->index()->with(['success' => 'Ignore directory was successfully updated']);
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
			$selectedIds = $request->input('selectedIds');
			foreach( $selectedIds as $id ) {
				App\FileIgnoreDir::destroy($id);
			}
			return $this->index()->with(['success' => 'Ignore directories were successfully deleted']);
		}

}
