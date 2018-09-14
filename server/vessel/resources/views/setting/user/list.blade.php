@extends('layouts.app')

@section ('scripts')
	<script src="{{ asset('js/tablesort.min.js') }}"></script>
	<script>
		$(document).ready( function() {

			$('.message .close')
				.on('click', function() {
					$(this)
						.closest('.message')
						.transition('fade');
				});

			$('.menu .item').tab();

			var list_form = $('#list_form');

			$('table').tablesort();

			$('#delete_selected').click(function() {
				$('input[name="_method"]').val('POST');
				list_form.attr('action', '{{ route('user_setting.destroyMultiple') }}');
				list_form.submit();
			});

			$('#add_setting').click(function(e) {
				e.preventDefault();
				window.location.href= '{{ route('user_setting.create') }}';
			});

		});
	</script>
@endsection

@section('content')

	<div class="ui very padded container">

		@if(isset($success))
		<div class="ui positive message">
			<i class="close icon"></i>
			<div class="header">
				{{ $success }}
			</div>
		</div>
		@endif

		@if(isset($error))
		<div class="ui negative message">
			<i class="close icon"></i>
			<div class="header">
				{{ $error }}
			</div>
		</div>
		@endif

		<h2 class="ui header">
			User Settings
			<div class="sub header">View and Manage User Specific Settings</div>
		</h2>
	</div>

	<form id="list_form" action="" method="POST">
		@csrf
		<input type="hidden" name="_method" value="post" />

		<div class="ui very padded container">
			<table class="ui selectable sortable stackable padded striped table">
				<thead>
					<tr>
						<th class="no-sort" colspan="8">
			      	{{ $userSettings->links() }}
			    	</th>
				  </tr>
					<tr>
						<th class="no-sort"></th>
						<th>Setting</th>
						<th>User</th>
						<th>Value</th>
					</tr>
				</thead>
				<tbody>
					@foreach ($userSettings as $setting)
						<tr>
							<td>
								<div class="ui fitted checkbox">
				          <input name="selectedIds[]" type="checkbox" value="{{ $setting->id }}"><label></label>
				        </div>
							</td>
							<td><a href="{{ route('user_setting.show', $setting->id) }}">{{ $setting->setting->name }}</td>
							<td>{{ $setting->user->first_name }} {{ $setting->user->last_name}}</td>
							<td>{{ $setting->value }}</td>
						</tr>
					@endforeach
				</tbody>
				<tfoot>
			    <tr>
						<th colspan="8">
							<div class="ui">
								<button id="add_setting" class="ui primary button">
									<i class="icon plus"></i>
								  Add
								</button>
								<button id="delete_selected" class="red ui button">
									<i class="icon delete"></i>
								  Delete
								</button>
							</div>
				      {{ $userSettings->links() }}
			    	</th>
				  </tr>
				</tfoot>
			</table>
		</div>

	</form>

@endsection
