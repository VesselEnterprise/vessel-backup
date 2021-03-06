<?php

namespace App\Http\Controllers;

use App\Deployment;
use Illuminate\Http\Request;
use Illuminate\Support\Carbon;

class DeploymentController extends Controller
{

    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
				$deployments = Deployment::paginate();
        return view('deployment.list', ['deployments' => $deployments]);
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
        return view('deployment.create', ['deploymentKey' => Deployment::generateToken()]);
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {
        $deployment = new Deployment();
				$deployment->deployment_name = $request->input('deployment_name');
				$deployment->expires_at = Carbon::parse( $request->input('expires_at') )->setTimezone('UTC');
				$deployment->deployment_key = $request->input('deployment_key');
				$deployment->never_expires = filter_var($request->input('never_expires'), FILTER_VALIDATE_BOOLEAN);
				$deployment->save();

				return $this->index();
    }

    /**
     * Display the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function show($id)
    {
        $deployment = Deployment::find($id);
				return view('deployment.show', ['deployment' => $deployment]);
    }

    /**
     * Show the form for editing the specified resource.
     *
     * @param  int  $id
     * @return \Illuminate\Http\Response
     */
    public function edit($id)
    {
				$deployment = Deployment::find($id);
				return view('deployment.show', ['deployment' => $deployment]);
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
        $deployment = Deployment::find($id);
				$deployment->deployment_name = $request->input('deployment_name');
				$deployment->expires_at = Carbon::parse( $request->input('expires_at') )->setTimezone('UTC');
				$deployment->never_expires = filter_var($request->input('never_expires'), FILTER_VALIDATE_BOOLEAN);
				$deployment->save();

				return $this->index();
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
				App\Deployment::withUuid($id)->destroy();
			}
			return $this->index()->with(['success' => 'Deployments were successfully deleted']);
		}
}
