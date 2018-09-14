@extends('layouts.app')

@section('scripts')
	<script>
		$(document).ready( function() {

			$('.ui.form')
			  .form({
			    fields: {
			      setting: {
			        identifier: 'setting',
			        rules: [
			          {
			            type   : 'empty',
			            prompt : 'Please select a setting'
			          }
			        ]
			      },
			      user: {
			        identifier: 'user',
			        rules: [
			          {
			            type   : 'empty',
			            prompt : 'Please specify a user'
			          }
			        ]
			      },
						value: {
			        identifier: 'value',
			        rules: [
			          {
			            type   : 'empty',
			            prompt : 'Please specify a value'
			          }
			        ]
			      },
			    }
			  });

			$('.message .close')
				.on('click', function() {
					$(this)
						.closest('.message')
						.transition('fade')
					;
				})
			;
		});
	</script>
@endsection

@section('content')

<div class="ui very padded container">

	<h2 class="ui header">
		Add New User Setting
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

	<form method="POST" action="{{ route('user_setting.store') }}" class="ui form segment">
		@csrf
		<div class="ui centered stackable grid container">

			  <div class="eight wide column field">
			    <label>Setting</label>
					<select name="setting" id="setting" class="ui dropdown" required>
						@foreach($settings as $setting)
							<option class="item" value="{{ $setting->setting_id }}">{{ $setting->name }}</option>
						@endforeach
					</select>
			  </div>

			  <div class="eight wide column field">
			    <label>User</label>
			    <select id="user" name="user" class="ui search dropdown" required>
						@foreach($users as $user)
							<option value="{{ $user->uuid_text }}">{{ $user->first_name }} {{$user->last_name }}</option>
						@endforeach
					</select>
			  </div>

			<div class="left floated eight wide column">
				<label>Value</label>
		    <input type="text" id="value" name="value" placeholder="Enter Value" required>
			</div>

			<div class="row">
				<div class="eight wide centered column">
					<div class="ui error message"></div>
				</div>
			</div>
			<div class="row">
				<div class="two wide centered column">
				  <div class="ui primary submit button">{{ __('Add') }}</div>
				</div>
			</div>

		</div>
	</form>

</div>
@endsection
