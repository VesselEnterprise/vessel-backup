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

		});
	</script>
@endsection

@section('content')

<div class="ui container">

	<h2 class="ui header">
		Manage Configuration
		<div class="sub header">Manage Application and Server settings</div>
	</h2>

	<form class="ui form" action="{{ route('setting.updateAll') }}" method="POST">
		@csrf
		<input type="hidden" name="_method" value="put" />

		<div class="ui stackable grid container segment">

			@foreach($settings as $setting)
			<div class="eight wide column">
				<div class="field">
					<label>{{ $setting->display_name }}</label>
					<input name="{{ $setting->setting_name }}" id="{{ $setting->setting_name }}" type="{{ $setting->data_type == 'integer' ? 'number' : 'text'}}" value="{{ $setting->value }}" placeholder="">
				</div>
			</div>
			@endforeach

			<div class="center aligned sixteen wide column">
				<div class="ui primary submit button">{{ __('Save') }}</div>
			</div>

		</div>

	</form>

</div>

@endsection
