@extends('layouts.app')

@section ('scripts')
	<script src="{{ asset('js/tablesort.min.js') }}"></script>
	<script>
		$(document).ready( function() {
			$('table').tablesort();
		});
	</script>
@endsection

@section('content')

	<div class="ui very padded container">
		<h2 class="ui header">
			Users
			<div class="sub header">View or Manage users</div>
		</h2>
	</div>

	<div class="ui very padded container">
		<table class="ui selectable sortable stackable padded striped table">
			<thead>
				<tr>
					<th colspan="7">
		      	{{ $users->links() }}
		    	</th>
			  </tr>
				<tr>
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
					<th colspan="7">
			      {{ $users->links() }}
		    	</th>
			  </tr>
			</tfoot>
		</table>
	</div>

@endsection
