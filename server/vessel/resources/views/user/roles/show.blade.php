@extends('layouts.app')

@section('scripts')
	<script>
		$(document).ready( function() {

			$('.ui.form')
			  .form({
			    fields: {
			      setting: {
			        identifier: 'role',
			        rules: [
			          {
			            type   : 'empty',
			            prompt : 'Please select a role'
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

				$('#delete_record').click(function() {
					$('input[name="_method"]').val('DELETE');
					list_form.submit();
				});

				$('#role').dropdown('set selected', '{{ $userRole->role_id }}');
				$('#user').dropdown('set selected', '{{ App\User::decodeUuid($userRole->user_id) }}');
		});
	</script>
@endsection

@section('content')

<div class="ui very padded container">

	<h2 class="ui header">
		Manage User Role
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

	<form method="POST" action="{{ route('user_role.update', $userRole->id) }}" class="ui form segment">
		@csrf
		<input type="hidden" name="_method" value="PUT" />

		<div class="ui centered stackable grid container">

		  <div class="eight wide column field">
		    <label>Role</label>
				<select name="role" id="role" class="ui dropdown" required>
					@foreach($roles as $role)
						<option class="item" value="{{ $role->id }}">{{ $role->name }}</option>
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

			<div class="row">
				<div class="eight wide centered column">
					<div class="ui error message"></div>
				</div>
			</div>
			<div class="row">
				<div class="three wide column">
				  <div class="ui primary submit button">{{ __('Update') }}</div>
				</div>
				<div class="three wide column">
					<div id="delete_record" class="ui red submit button">
						<i class="icon delete"></i>
						Delete
					</div>
				</div>
			</div>

		</div>
	</form>

</div>
@endsection
