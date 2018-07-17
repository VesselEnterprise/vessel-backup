@extends('layouts.app')

@section ('scripts')
	<script src="{{ asset('js/tablesort.min.js') }}"></script>
	<script>
		$(document).ready( function() {
			$('table').tablesort();

			$('#create_user').click(function(e) {
				e.preventDefault();
				window.location.href= '{{ url('user/create') }}';
			});

		});
	</script>
@endsection

@section('content')

	<div class="ui very padded container">
		<h2 class="ui header">
			Users
			<div class="sub header">View and Manage users</div>
		</h2>
	</div>

	<div class="ui very padded container">
		<table class="ui selectable sortable stackable padded striped table">
			<thead>
				<tr>
					<th class="no-sort" colspan="8">
		      	{{ $users->links() }}
		    	</th>
			  </tr>
				<tr>
					<th class="no-sort"></th>
					<th>E-Mail</th>
					<th>Username</th>
					<th>First name</th>
					<th>Last Name</th>
					<th>Office</th>
					<th>Last Login</th>
					<th>Last Backup</td>
				</tr>
			</thead>
			<tbody>
				@foreach ($users as $user)
					<tr>
						<td>
							<div class="ui fitted checkbox">
			          <input type="checkbox"><label></label>
			        </div>
						</td>
						<td><a href="{{ route('user.show', $user->uuid_text) }}">{{ $user->email }}</a></td>
						<td>{{ $user->user_name }}</td>
						<td>{{ $user->first_name }}</td>
						<td>{{ $user->last_name }}</td>
						<td>{{ $user->office }}</td>
						<td>{{ $user->last_login }}</td>
						<td>{{ $user->last_login }}</td>
					</tr>
				@endforeach
			</tbody>
			<tfoot>
		    <tr>
					<th colspan="8">
						<div class="ui">
							<button id="create_user" class="ui primary button">
								<i class="icon user plus"></i>
							  Create New
							</button>
							<button class="red ui button">
								<i class="icon trash"></i>
							  Delete
							</button>
							<button class="ui button">
								<i class="icon lock"></i>
							  Deactivate
							</button>
						</div>
			      {{ $users->links() }}
		    	</th>
			  </tr>
			</tfoot>
		</table>
	</div>

@endsection
