@extends('layouts.app')

@section('scripts')
	<script>
		$(document).ready( function() {

			$('.ui.form')
			  .form({
			    fields: {
			      setting: {
			        value: 'value',
			        rules: [
			          {
			            type   : 'empty',
			            prompt : 'Please select a setting'
			          }
			        ]
			      }
			    }
			  });

			$('.message .close')
				.on('click', function() {
					$(this)
						.closest('.message')
						.transition('fade')
					;
				});

		});
	</script>
@endsection

@section('content')

<div class="ui very padded container">

	<h2 class="ui header">
		Edit Setting: {{ $setting->setting_name }}
		<div class="sub header"></div>
	</h2>

	@if( isset($error) )
		<div class="ui negative message">
			<i class="close icon"></i>
			<div class="header">
				{{ $error }}
			</div>
		</div>
	@endif

	<form method="POST" action="{{ route('setting.update', $setting->setting_id) }}" class="ui form segment">
		@csrf
		<input type="hidden" name="_method" value="PUT" />

		<div class="ui centered stackable grid container">

			<div class="left floated eight wide column">
				<label>{{ $setting->setting_name }}</label>
		    <input type="text" id="value" name="value" placeholder="Enter Value" value="{{ $setting->value }}" required>
			</div>

			<div class="row">
				<div class="eight wide centered column">
					<div class="ui error message"></div>
				</div>
			</div>
			<div class="row">
				<div class="three wide column">
				  <div class="ui primary submit button">{{ __('Update') }}</div>
				</div>
			</div>

		</div>
	</form>

</div>
@endsection
