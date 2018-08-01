@extends('layouts.app')

@section('scripts')
	<script>
		$(document).ready( function() {

			$('.ui.form').form();

			$('.message .close')
				.on('click', function() {
					$(this)
						.closest('.message')
						.transition('fade');
				});

			$('.menu .item').tab();

			$('#expires_at').calendar();

		});
	</script>
@endsection

@section('content')

<div class="ui container">

	<h2 class="ui header">
		Create New Deployment
		<div class="sub header">Add a new deployment to enable the client for multiple users</div>
	</h2>

	<form class="ui form" action="{{ route('deployment.store') }}" method="POST">
		@csrf

		<div class="ui stackable grid container segment">

			<div class="eight wide column">
				<div class="field">
					<label>Deployment Name</label>
				  <input name="deployment_name" id="deployment_name" type="text" placeholder="Enter any name">
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Expires at</label>
					<div class="ui calendar" id="expires_at">
						<div class="ui input left icon">
							<i class="calendar icon"></i>
							<input name="expires_at" type="text" placeholder="Date/Time">
						</div>
					</div>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Deployment Key</label>
					<input name="deployment_key" id="deployment_key" type="text" value="{{ $deploymentKey }}" placeholder="">
				</div>
			</div>

			<div class="center aligned sixteen wide column">
				<div class="ui primary submit button">{{ __('Submit') }}</div>
			</div>

		</div>

	</form>

</div>

@endsection
