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
				list_form.attr('action', '{{ route('deployment.destroyMultiple') }}');
				list_form.submit();
			});

			$('#create_deployment').click(function(e) {
				e.preventDefault();
				window.location.href= '{{ url('deployment/create') }}';
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
			Deployments
			<div class="sub header">View and Manage Application Deployments</div>
		</h2>
	</div>

	<form id="list_form" action="" method="POST">
		@csrf

		<div class="ui very padded container">
			<table class="ui selectable sortable stackable padded striped table">
				<thead>
					<tr>
						<th class="no-sort" colspan="4">
			      	{{ $deployments->links() }}
			    	</th>
				  </tr>
					<tr>
						<th class="no-sort"></th>
						<th>Name</th>
						<th>Expires at</th>
						<th>Created at</th>
					</tr>
				</thead>
				<tbody>
					@foreach ($deployments as $deployment)
						<tr>
							<td>
								<div class="ui fitted checkbox">
				          <input name="selectedIds[]" type="checkbox" value="{{ $deployment->id }}"><label></label>
				        </div>
							</td>
							<td><a href="{{ route('deployment.show', $deployment->id) }}">{{ $deployment->deployment_name }}</td>
							<td>{{ $deployment->expires_at }}</td>
							<td>{{ $deployment->created_at }}</td>
						</tr>
					@endforeach
				</tbody>
				<tfoot>
			    <tr>
						<th colspan="9">
							<div class="ui">
								<button id="create_deployment" class="ui primary button">
									<i class="icon plus"></i>
								  Create New
								</button>
								<button id="delete_selected" class="red ui button">
									<i class="icon delete"></i>
								  Delete
								</button>
							</div>
				      {{ $deployments->links() }}
			    	</th>
				  </tr>
				</tfoot>
			</table>
		</div>

	</form>

@endsection
