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
				list_form.attr('action', '{{ route('user_role.destroyMultiple') }}');
				list_form.submit();
			});

			$('#add_role').click(function(e) {
				e.preventDefault();
				window.location.href= '{{ route('user_role.create') }}';
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
			User Roles
			<div class="sub header">View and Manage User Roles</div>
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
			      	{{ $userRoles->links() }}
			    	</th>
				  </tr>
					<tr>
						<th class="no-sort"></th>
						<th>Role</th>
						<th>User</th>
					</tr>
				</thead>
				<tbody>
					@foreach ($userRoles as $role)
						<tr>
							<td>
								<div class="ui fitted checkbox">
				          <input name="selectedIds[]" type="checkbox" value="{{ $role->id }}"><label></label>
				        </div>
							</td>
							<td><a href="{{ route('user_role.show', $role->id) }}">{{ $role->role->name }}</td>
							<td>{{ $role->user->first_name }} {{ $role->user->last_name }}</td>
						</tr>
					@endforeach
				</tbody>
				<tfoot>
			    <tr>
						<th colspan="8">
							<div class="ui">
								<button id="add_role" class="ui primary button">
									<i class="icon plus"></i>
								  Add
								</button>
								<button id="delete_selected" class="red ui button">
									<i class="icon delete"></i>
								  Delete
								</button>
							</div>
				      {{ $userRoles->links() }}
			    	</th>
				  </tr>
				</tfoot>
			</table>
		</div>

	</form>

@endsection
