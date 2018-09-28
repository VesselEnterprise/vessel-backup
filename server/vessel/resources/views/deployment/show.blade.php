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
			$('.never-expires').checkbox('{{ $deployment->never_expires ? 'check' : 'uncheck' }}');

		});
	</script>
@endsection

@section('content')

<div class="ui container">

	<h2 class="ui header">
		Manage Deployment
		<div class="sub header">Manage an existing deployment</div>
	</h2>

	<form class="ui form" action="{{ route('deployment.update', ['id' => $deployment->id]) }}" method="POST">
		@csrf
		<input type="hidden" name="_method" value="put" />

		<div class="ui stackable grid container segment">

			<div class="eight wide column">
				<div class="field">
					<label>Deployment Name</label>
				  <input name="deployment_name" id="deployment_name" value="{{ $deployment->deployment_name }}" type="text" placeholder="Enter any name">
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Expires at</label>
					<div class="ui calendar" id="expires_at">
						<div class="ui input left icon">
							<i class="calendar icon"></i>
							<input name="expires_at" type="text" value="{{ $deployment->expires_at }}" placeholder="Date/Time">
						</div>
					</div>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Deployment Key</label>
					<input name="deployment_key" id="deployment_key" type="text" value="{{ $deployment->deployment_key }}" placeholder="" readonly>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<div class="ui checkbox never-expires">
					  <input type="checkbox" name="never_expires" id="never_expires">
					  <label>Never Expires</label>
					</div>
				</div>
			</div>

			<div class="center aligned sixteen wide column">
				<div class="ui primary submit button">{{ __('Save') }}</div>
			</div>

		</div>

	</form>

</div>

@endsection
